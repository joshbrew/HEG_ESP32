import serial, traceback, threading, time, sys, struct, os, array
import zipfile, shelve, random, binascii
import cPickle as pickle
import fakeserial
import itertools


################################################################################
# parsers.py
# A fairly lightweight threaded library for parsing HEG input from Pocket 
# Neurobics devices and from the Peanut.
#
# Copyright 2012, Jonathan Toomim (jtoomim@jtoomim.org)
# This software is available under the terms of the GNU Lesser Public License,
# version 3.0. For a copy of this license, see the file LGPL.txt, or visit 
# http://www.gnu.org/licenses/lgpl-3.0.txt.
#
# A copy of the specification for one of the protocols implemented herein can 
# be found at http://www.pocket-neurobics.com/simplex_spec.htm.
# This is not a complete implementation of the protocol. This is not an elegant
# implementation of the protocol. This is a good enough implementation of the 
# protocol for the uses of software I've written.
#
# The Peanut protocol is currently undocumented. Read the code or bug me.
#
# Want to adapt this software for your project? Want to make your biofeedback
# hardware supported by this software? Please do! Email me, and I might even
# help.
################################################################################

__VERSION__ = version = '0.4.8b + HEGduino'

DEBUG = '--debug' in sys.argv
ENABLE_PNB = '--enable-pnb' in sys.argv or 'enable-pnb' in os.listdir('..')
file_extensions = ('pnb', 'pea', 'heg', 'heg.zip')

tmp_path = '.'

def make_zip(archive, rawdata, timing, *files, **kwargs):
    if not 'overwrite' in kwargs:        kwargs['overwrite'] = False
    if not 'remove_originals' in kwargs: kwargs['remove_originals'] = True
    if not 'strings' in kwargs:          kwargs['strings'] = {}
    if os.path.exists(archive) and not kwargs['overwrite']:
        print "Path %s exists. Not overwriting." % archive
        raise IOError
    if os.path.exists(archive) and kwargs['overwrite']: 
        if DEBUG: print "Path %s exists. Overwriting." % archive
        os.unlink(archive)
    #try:
    zip = zipfile.ZipFile(archive, mode='w', compression=zipfile.ZIP_DEFLATED)
    zip.write(rawdata, 'rawdata')
    zip.write(timing, 'timing')
    
    for f in files:
        zip.write(f)
    for k,v in kwargs['strings'].items():
        zip.writestr(k,v)
    
    zip.close()
    #except:
    #    zip.close()
    #    raise IOError

    # Still with us? Good. Then let's...
    if kwargs['remove_originals']:
        for f in (rawdata, timing) + files:
            try:
                os.unlink(f)
            except:
                if DEBUG:
                    traceback.print_exc()
    return

    
def is_heg_archive(path):
    if hasattr(path, 'endswith'): return path.lower().endswith('.heg') or path.lower().endswith('heg.zip')
    else: return False # fixme for file handles

def get_rawdata_handle(path):
    try:
        with zipfile.ZipFile(path) as zip:
            rawdata = zip.open('rawdata')
        return rawdata
    except:
        if DEBUG: traceback.print_exc()
        return None
    
    
    
def enumerate_serial_ports():
    if sys.platform == 'win32':
        import _winreg as winreg

        path = 'HARDWARE\\\\DEVICEMAP\\SERIALCOMM'
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
        except WindowsError:
            return []
        
        i = 0
        comports = []
        while 1:
            try:
                val = winreg.EnumValue(key, i)
                comports.append((str(val[1]), str(val[0])))
                i += 1
            except EnvironmentError:
                break
                    
    elif sys.platform == 'darwin':
        comports = [('/dev/'+d, d) for d in os.listdir('/dev/') if d.startswith('cu.usbserial')]    
    elif sys.platform == 'linux':
        comports = [('/dev/'+d, d) for d in os.listdir('/dev/') if d.startswith('ttyUSB')]
    else:
        comports = []
    return comports

def detect_protocols(ports=None):
    if ports==None:
        ports = enumerate_serial_ports()
        if sys.platform == 'win32':
            ports = [port[0] for port in ports if 'VCP' in port[1] or 'USBSER' in port[1] or 'Silabser' in port[1]]            
        else:
            ports = [port[0] for port in ports]
    threads = []
    for port in ports:
        threads.append(ProtocolDetector(port=port))
        threads[-1].start()
    for t in threads:
        t.join()
    results = [(t.port, t.protocol) for t in threads if t.protocol != None]# and t.protocol != 'no data' and t.protocol != 'no connect']
    return results
    
def export_csv(source, dest):
    heg = AutoHEG(port=source, fast=True)
    heg.start()
    heg.join()
    if heg.mgr: heg.mgr.join()
    else:
        if DEBUG: print "No mgr to join in parsers.export_csv()"
        return heg.Close()
    if DEBUG: print heg.detector.protocol
    if heg.detector.protocol == 'pnb':
        columns = ['sample', 'HEG Ratio']
        outfile = file(dest, 'w')
        outfile.write([','.join(i, h) for i,h in zip(range(len(heg.hegdata)), heg.hegdata)] + '\n')
        outifle.close()
        heg.Close()
    else:
        if 'rawdata6' in heg.data.keys():
            rd6 = heg.data['rawdata6']
            rds = {}
            for i in range(6):
                rds['rawdata6_%i' % i] = [v[i] for v in rd6]
            rds['IR']  = [ ir - (1 +  air1 +  air2)/2 \
                for  ir, air1,  air2 in zip(rds['rawdata6_1'], rds['rawdata6_0'], rds['rawdata6_2'])]
            rds['RED'] = [red - (1 + ared1 + ared2)/2 \
                for red, ared1, ared2 in zip(rds['rawdata6_4'], rds['rawdata6_3'], rds['rawdata6_5'])]
            del heg.data['rawdata6']
            heg.data.update(rds)

        if DEBUG: 
            print heg.data.keys()
            print len(heg.times)
        keys = heg.data.keys()
        keys.sort()
        keys = ['timestamp', 'session time'] + keys
        vals = {}
        vals.update(heg.data)
        vals['timestamp'] = heg.times
        vals['session time'] = [t - heg.times[0] for t in heg.times]
        outfile = file(dest, 'w')
        outfile.write(','.join(keys) + '\n')
        for i in range(len(heg.times)):
            outfile.write(','.join([str(vals[k][i]).replace(',', '\\;')  for k in keys]) + '\n')
#        for d,t in zip(heg.dicts, heg.times):
#            outfile.write(','.join([`t`] + 
#                                   [`t - heg.times[0]`] +
#                                   [`d[col]`.replace(',', '\\;') if col in d else '' for col in columns[2:]]) + '\n')
        outfile.close()
        heg.Close()

def linear_FIR_filter_factory(taps):
    def LinearFIRFilter(data):
        if taps > len(data):
            newtaps = len(data)
        else:
            newtaps = taps
        newdata = data[-newtaps:]
        result = 0.
        for i in range(newtaps):
            result += (i+.5) * newdata[i]
        return 2.*result/float(newtaps*newtaps)
    return LinearFIRFilter

class ProtocolDetector(threading.Thread):
    def __init__(self, port, baudrate, timeout):
        print 'Detecting Protocol'
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.protocol = None
        if baudrate == 115200:
            self.data = []
        else:
            self.data = ''
        threading.Thread.__init__(self)
            
    def run(self):
        try:
            port = self.port
            if is_heg_archive(port):
                s = get_rawdata_handle(port) # s is for serial port, which this imitates
            else:
                if type(port) == int: port -= 1
                if hasattr(port, 'read'):
                    s = port
                elif type(port) in (str, unicode) and not port.startswith('/dev') and not (port.startswith('COM') and port[3:].isdigit()):
                    s = fakeserial.FakeSerial(port, 'rb', byterate=0)
                else:
                    print 'getting Serial connection on port: ', port
                    print 'Baudrate: ', self.baudrate
                    s = serial.Serial(port=port, baudrate=self.baudrate, timeout=2)
                    if self.baudrate == 38400:
                        s.write('protocol 3\n')
                    elif self.baudrate == 115200:
                        time.sleep(1)
                        self.protocol = 'hegduino'
                        print 'HEGduino connected'
                        s.write('t')
                    #s.flush()
                    #s.flushInput()
            if self.baudrate == 115200:
                print 'read initial bytes'
                for i in range(1,20):
                    temp = s.readline()
                    print temp

            if self.baudrate == 38400:
                #s.flushInput()
                print 'read initial bytes'
                self.data += s.read(200)
                data = self.data
                print data
        except Exception, e:
            print e
            try:
                if DEBUG: print "Serial exception when detecting ", self.port
                if DEBUG: print "Serial port is ", s
                s.close()
            except:
                pass
            self.protocol = 'no connect'
            print self.protocol
            try: s.close()
            except: pass
            return
        data = []
        if self.baudrate == 115200:
            while(len(self.data) < 10):
                temp = s.readline()
                if temp.find(','):
                    temp = temp.split(',')
                    if len(temp) > 1 and temp[1].find('WAIT') == -1:
                        self.data.append(float(temp[1]))
                    #self.data.append(temp[0])
                    data = self.data
                    if len(self.data) < 1:
                        #if float(temp[0]) < 7000:
                        print 'Getting Baseline '
                        #else:
                        #    print 'Light level too high, check sensor'
                    print data, '\n'
                else:
                    print 'Callback: ', temp
        
        if not data:
            self.protocol = 'no data'
            print self.protocol
            s.close()
            return

        if self.baudrate == 38400:
            while len(self.data) < 1024:
                try:
                    if self.baudrate == 38400:
                        self.data += s.read(20)
                        data = self.data
                        print data
                except serial.SerialException:
                    try:
                        if DEBUG: print "Serial exception when detecting ", self.port
                        s.close()
                    except:
                        pass
                    self.protocol = 'no connect'
                    s.close()
                    return
                if self.baudrate == 38400:
                    peanutscore = data.count('\xaa\xaa')
                    print 'Peanut score: ',peanutscore
                    asciiscore  = data.count('\n1, 0x')
                    i, j, pnbscore = 0, 0, 0
                    d = map(ord, data)
                    while i < len(d)-20:
                        if d[i]>>6 == 0 and d[i+5]>>6 == 1 and d[i+10]>>6 == 2 and d[i+15]>>6 == 3:
                            pnbscore += 1
                            i += 20
                        else:
                            i += 1
                    if ENABLE_PNB:
                        while i < len(data):
                            if not ord(data[i])>>6 == j:
                                i += 1
                                j = 0
                            else:
                                i += 5
                                j += 1
                                if j == 4:
                                    j = 0
                                    pnbscore += 1
                        if DEBUG: print "pnbscore: %i, asciiscore: %i, peanutscore: %i, len(data): %i" % (pnbscore, asciiscore, peanutscore, len(data))
                #if asciiscore >= peanutscore*4 + 4 and asciiscore >= pnbscore + 4:
                #     self.protocol = 'ascii'
                #     break
                    if pnbscore >= peanutscore*4 + 4:# and pnbscore >= asciiscore + 2:
                        self.protocol = 'pnb'
                        break
                    elif peanutscore*4 >= asciiscore + 4:# and peanutscore >= pnbscore + 3:
                        self.protocol = 'peanut'
                        break
                    else:
                        self.protocol = 'unknown'
                    if DEBUG: print "pnbscore: %i, asciiscore: %i, peanutscore: %i, len(data): %i" % (pnbscore, asciiscore, peanutscore, len(data))
        if self.baudrate == 115200:
            s.write('f')
        s.close()
        print 'Protocol detected: ', self.protocol
        return


class SerialManager(threading.Thread):
    packet_length_pos = 0
    file_extension = 'pna'
    default_packet_length = 6
    timeout_duration = .6
    timeout_count = 3
    def __init__(self, baudrate, port=None, callback=lambda x: x, rawlog=False, skip=0, fast=False, pause=False):
        threading.Thread.__init__(self)
        
        time1 = self.starttime = time.time()
        self.port, self.fast, self.skip, self.callback, self.paused = port, fast, skip, callback, pause
        self.ser = None
        self.baudrate = baudrate
        self.connected, self.keeprunning = False, False
        self.timeouts = 0
        self.read_str = ''
        self.rawdata = ''
        self.rawindex    = 0
        self.parseerrors = 0
        self.heg_raw = []
        self.heg_sma = []
        self.hegdata     = [] # this array goes to the graph
        self.unfilteredhegdata = []
        self.data        = {}
        self.dicts       = []
        self.parseddata  = []
        self.times       = []
        self.rawlogfilename = u''
        self.metadata    = {}
        self.metarev     = 0
        self.metaCRC     = None
        self.metafile    = u''
        
        self.samplerate = 1./.008192/4. # ~30 Hz, good for PNB, bad for peanut. FIXME

        print 'Serial Thread Initialized'
        if type(port) in (str, unicode) and not port.startswith('/dev') and not port.upper().startswith('COM'):
            self.live = False
            rawlog    = False
        else: 
            self.live = True
            self.fast = 0
        
        self.timingMemory = fakeserial.RememberSerialTiming(name=None)

        if rawlog: self.LogRawToFile(rawlog)
        else:      self.rawlog = None
        self.logtiming = False
        
    
        
    def LogRawToFile(self, filename, timing=True, append_extension=True, move_old_log=True):
        if append_extension and not filename.lower().endswith(self.file_extension):
            filename += u'.' + self.file_extension
        self.rawlogfilename = filename
        
        if DEBUG: print u"filename type is ", type(filename)
        if DEBUG: print map(ord, filename)

        if move_old_log and self.rawlogfilename and timing and \
            hasattr(self, 'rawlog') and type(self.rawlog) in (fakeserial.DumpSerialTiming, fakeserial.SerialTiming):
            self.rawlog.rename(filename)
            #self.logtiming = timing # shouldn't be necessary since we checked type(self.rawlog)
            return

        #if DEBUG: print "Logging to %s" % filename
        self.EndRawLog()
        write_mode = 'ab' if os.path.exists(filename) else 'wb'
        if timing:
            self.rawlog = fakeserial.SerialTiming(filename, write_mode, starttime=self.starttime,
                                                      prependdata=self.rawdata, 
                                                      prependtiming=self.timingMemory.timingfile.getvalue())
        else:
            self.rawlog = open(filename, write_mode)
            self.rawlog.write(self.rawdata)
        #self.rawlog.write(self.rawdata)
        self.logtiming = timing
        
        if hasattr(self, 'metadata') and type(self.metadata) == dict:
            tmp = self.metadata
            self.GetMetadata()
            self.metadata.update(tmp)
        else:
            self.GetMetadata()
    
    def EndRawLog(self):
        try:    self.rawlog.close()
        except: pass
        self.rawlog = None
        
    def join(self, *args, **kwargs):
        if self.live:
            self.keeprunning = False
        threading.Thread.join(self, *args, **kwargs)
   
    def ReadData(self, n=0):
        try:
            data = self.ser.read(n)
            self.rawdata += data
            if not self.paused:
                self.timingMemory.write(data)
            if self.rawlog and not self.paused:
                self.rawlog.write(data)
            if not data and not self.live:
                self.keeprunning = False
            return data
        except EOFError:
            self.keeprunning = False
        except ValueError:
            self.keeprunning = False
        except serial.SerialException:
            if DEBUG and self.keeprunning:
                print "Serial exception reading %i bytes. Traceback:" % n
                traceback.print_exc()
        except:
            if DEBUG and self.keeprunning:
                print "Couldn't read %i bytes. Traceback:" % n
                traceback.print_exc()
            self.ClosePort()
        return ''
    
    def FastReadData(self, n=0):
        #print "reading %i bytes; at %i of %i" % (n, self.rawindex, self.rawlen)
        if not n or self.rawindex+n >= self.rawlen:
            dat = self.rawdata[self.rawindex:self.rawlen]
            self.rawindex = self.rawlen
            self.keeprunning = False
            return dat
        else:
            dat = self.rawdata[self.rawindex:self.rawindex+n]
            self.rawindex += n
            return dat
    
    def GetHandlesFromZipfile(self, path):
        # should add error checking here
        self.zfpath = path
        self.zf = zf = zipfile.ZipFile(path)
        self.hegfile = zf.open('rawdata', 'r')       
        self.tmgfile = None
        contents = zf.infolist()
        
        # workaround for lack of seek() in zipfile files
        for info in contents:
            if info.filename == 'rawdata':
                self.hegfile.length = info.file_size
            if info.filename == 'timing':
                self.tmgfile = zf.open('timing', 'r')
            if info.filename == 'hegarray':
                self.hegafile = zf.open('hegarray', 'r')
            if info.filename == 'timesarray':
                self.tmgafile = zf.open('timesarray', 'r')
        
        metafile = None
        metarev  = 0
        metaCRC  = None
        for info in contents:
            if info.filename.startswith('metadata'):
                try:
                    x = int(info.filename.split('metadata')[1].split('.')[0])
                except:
                    if DEBUG: traceback.print_exc()
                    x = 1
                if x > metarev:
                    metarev = x
                    metafile = info.filename
                    metaCRC  = info.CRC
        if metafile:
            mf = zf.open(metafile)
            self.metadata = pickle.load(mf)
            mf.close()
            self.metarev = metarev
        else:
            self.metadata = {}
            #
            #self.metafile = path + '.metadata%i.db' % metarev
            #if not os.path.exists(self.metafile):
            #    if not os.path.exists(tmp_path): os.mkdir(tmp_path)
            #    newpath = os.path.join(tmp_path, 'tmp%09i' % random.randint(0, 2<<30))
            #    os.mkdir(newpath)
            #    zf.extract(metafile, newpath)
            #    os.rename(os.path.join(newpath, metafile), path + '.metadata%i.db' % metarev)
            #
            #self.metarev  = metarev
            #self.metaCRC  = metaCRC
        #else:
        #    self.metafile = path + '.metadata%i.db' % metarev
        #    #self.metarev == 0
        #    self.metaCRC = -1 # doesn't matter; updating the zipfile needs to happen.
        #self.metadata = shelve.open(self.metafile)
        zf.close()
        return self.hegfile, self.tmgfile, self.metadata

        
    def CreateZipFile(self, other_files=[], overwrite=True):
        if self.metafile and not self.metafile in other_files:
            other_files.append(self.metafile)
        self.zfpath = self.rawlogfilename[:-4]
        if not self.zfpath.lower().endswith('.heg'): self.zfpath +=  '.heg'
        
        strings = {}
        if self.hegdata and self.times:
            strings['hegarray']   = array.array('d', self.hegdata).tostring()
            strings['timesarray'] = array.array('d', self.times).tostring()
        
        return make_zip(self.zfpath,
                        self.rawlogfilename, self.rawlogfilename[:-4] + '.tmg', 
                        *other_files,
                        overwrite=overwrite, strings=strings)

    def MaybeUpdateMetadata(self, unlink=False):
        #return # FIXME: make this work
        #if self.metadata: pass#self.metadata.close()
        try:
            with zipfile.ZipFile(self.zfpath, 'r') as zf:
                with zf.open('metadata%i' % self.metarev) as mf:
                    dat = mf.read()
        except:
            dat = ''
        if not dat: dat = pickle.dumps({})
            
        md  = pickle.dumps(self.metadata)
        if not dat == md and hasattr(self, 'zfpath') and self.zfpath:
            self.metarev += 1
            with zipfile.ZipFile(self.zfpath, 'a') as zf:
                zf.writestr('metadata%i' % self.metarev, pickle.dumps(self.metadata))
               
#        try:
#            f = open(self.metafile)
#            dat = f.read()
#            f.close()
#        except:
#            try: f.close()
#            except: pass
#            return
#        CRC = binascii.crc32(dat) & 0xffffffff
#        if not CRC == self.metaCRC:
#            if DEBUG: print CRC, self.metaCRC
#            self.zf.close()
#            self.zf = zipfile.ZipFile(self.zfpath, 'a')
#            self.zf.write(self.metafile, 'metadata%i' % (self.metarev+1))
#            self.zf.close()
#        if unlink: os.unlink(self.metafile)
    
    def GetMetadata(self):
        if self.metadata: return self.metadata
        if self.live:# and self.rawlogfilename: # need to create the shelf
            self.metadata = {} #shelve.open(self.rawlogfilename + '.meta0.db')
            return self.metadata
        
        #if self.live and not self.rawlogfilename: 
        #    return {}
        else:
            raise ValueError # I hope this never gets here
        
        
    def InitSerialPort(self, evt=None):
        #if DEBUG: print "InitSerialPort called with port", self.port
        port, tmgfile = self.port, None
        if type(port) == int:
            port -= 1
        if not self.live:
            if is_heg_archive(port):
                port, tmgfile, metadata = self.GetHandlesFromZipfile(port)
            if self.fast == 3:
                if hasattr(self, 'hegafile') and hasattr(self, 'tmgafile'):
                    self.hegdata = array.array('d', self.hegafile.read())
                    self.times   = array.array('d', self.tmgafile.read())
                    self.keeprunning = False
                else:
                    self.fast = 2
                    
            if self.fast == 2:
                if hasattr(port, 'read'): 
                    self.ser = port
                else:
                    self.ser = open(port, 'rb')
                self.rawdata = self.ser.read()
                self.rawlen = len(self.rawdata)
                self.ser.close()
                self.ReadData = self.FastReadData
            elif self.fast:
                self.ser = fakeserial.FakeSerialTiming(port, 'rb', 
                                                       fast=True, 
                                                       timingfile=tmgfile)
            else:
                self.ser = fakeserial.FakeSerialTiming(port, 'rb', 
                                                       byterate=610., 
                                                       timingfile=tmgfile) 
            if self.skip:
                self.ser.seek(self.skip)
            
            print 'InitSerialPort: Pass'
            self.connected = True
            return True
        
        self.timeouts = 0
        try:
            #if DEBUG: print "Trying to connect to port ", port
            self.ser = serial.Serial(port=port, baudrate=self.baudrate, timeout=self.timeout_duration)
            first_response = ''
            if(self.baudrate == 115200):
                time.sleep(1)
                self.ser.write('t')
                for i in range(1,20):
                    print self.ser.readline()
                self.start_time = time.time()
                TIME_FORMAT = "%Y-%m-%d %H-%M-%S"
                start_time_str = time.strftime(TIME_FORMAT, time.localtime(self.start_time))
                #if DEBUG: print 'Connected at %s, "$$$" -> %s' % (start_time_str, first_response)
                self.connected = True
                print 'Init HEGduino port: Pass'
                print 'Waiting for Baseline...'
                return True
            else:
                first_response = self.ReadData(12)
            
            if first_response:
                self.read_str = first_response
                self.start_time = time.time()
                TIME_FORMAT = "%Y-%m-%d %H-%M-%S"
                start_time_str = time.strftime(TIME_FORMAT, time.localtime(self.start_time))
                #if DEBUG: print 'Connected at %s, "$$$" -> %s' % (start_time_str, first_response)
                self.connected = True
                print 'InitSerialPort: Pass'
                return True
            #elif DEBUG: print "First response:", first_response
        except:
            if DEBUG: traceback.print_exc()
            try:      self.ser.close()
            except:   pass
            print 'InitSerialPort: Fail'

            return False


    def FindPacketStart(self, pkt):
        return 0

    def IsComplete(self, pkt):
        return pkt.endswith('\n')
    
    def GuessPacketLength(self, pkt):
        return 0

    def Parse(self, pkt):
        return pkt
    
    def Read(self, n=0, timeout=.99):
        try:
            start_time = time.time()
            result = self.read_str
            self.read_str = ''
            if not self.connected:
                if DEBUG: print "Read() called when not self.connected."
                self.InitSerialPort()
            if self.connected:
                rem_len = self.default_packet_length - len(result)
                if rem_len > 0: result += self.ReadData(rem_len)
                while self.keeprunning:
                    if self.IsComplete(result):
                        return result
                    if  self.live and (time.time() - start_time > timeout):
                        if DEBUG: print 'ran out of time: %f' % (time.time() - start_time)
                        self.timeouts += 1
                        return result
                    start = self.FindPacketStart(result)
                    #if DEBUG and start: print 'start was ', start, 'on a len of %i on port %s' % (len(result), `self.port`)
                    if start == -1:
                        #result = result[-self.packet_length_pos+2:]
                        #result += self.ReadData(2)
                        if len(result) < self.default_packet_length:
                            if DEBUG: print 'need %i bytes, only have %i' % (self.default_packet_length, len(result))
                            if DEBUG: print 'connection is', self.connected, 'trying to read %i bytes' % (self.default_packet_length - len(result))
                            result += self.ReadData(self.default_packet_length - len(result))
                        else:
                            result = result[1:]
                            result += self.ReadData(1)
                        continue
                    elif start:
                        result = result[start:]
                        continue
                
                    if self.packet_length_pos > len(result):
                        result += self.ReadData(self.packet_length_pos - len(result))
                    pkt_len = self.GuessPacketLength(result)
                    if pkt_len and pkt_len > len(result): 
                        result += self.ReadData(pkt_len - len(result))
                    #and ((len(result) < n) or (not n)):
                    if not self.IsComplete(result):
                        result += self.ReadData(1)
                    if not result: 
                        if DEBUG: print 'no result'
                        self.timeouts += 1
                    else:
                        self.timeouts = 0
                return result
        except ValueError:
            if DEBUG: print "ValueError in Read(). Probably tried to use a closed port."
            self.ser.close()
            self.connected = False
            return ''
        except:
            if DEBUG: traceback.print_exc()
            try:
                self.ser.close()
            except:
                if DEBUG:
                    print "Couldn't close serial port after error. Further error:"
                    traceback.print_exc()
            self.connected = False
            return ''
    def write(self, txt):
        if self.live:
            self.ser.write(txt)
    def run(self): # needs to be lower-case for threading.Thread
        self.keeprunning = True
        while self.keeprunning:
            #if self.paused:
            #    time.sleep(.05)
            #elif not self.connected:
            if not self.connected:
                self.InitSerialPort()
                time.sleep(.1)
            else:
                try:
                    if self.baudrate == 115200:
                        newline = self.ser.readline()       
                    else:        
                        newline = self.Read()
                    try:
                        newdata = 0
                        if self.baudrate == 115200:
                            if len(newline) > 5 and len(newline) < 30:
                                temp = newline.split(',')
                                if temp[1].find('WAIT') == -1: # ADC,RATIO,POSITION_FROM_BASELINE. RATIO POSITION WILL READ "WAIT" BETWEEN RATIO READINGS
                                    newdata = float(temp[1])
                        else:
                            newdata = self.Parse(newline)
                    except:
                        #if DEBUG:
                        traceback.print_exc()
                        self.parseerrors += 1
                        continue
                    if newdata and not self.paused:
                        self.parseddata.append(newdata)
                        if self.fast == 2 or (self.fast and not self.ser.use_timing):
                            self.times.append(len(self.times)/self.samplerate)
                        elif self.live:
                            self.times.append(time.time())
                            #if self.logtiming: # FIXME: Check to see that this is unnecessary
                            #    self.rawlog.logtime()
                        else:
                            self.times.append(self.ser.getTime())
                            if len(self.times) > 2 and self.times[-1] < self.times[-2]: self.times[-1] = self.times[-2]
                        self.callback(newdata)
                    if self.timeouts >= self.timeout_count:
                        self.ClosePort()
                        self.InitSerialPort()
                    self.parseerrors = max(0, self.parseerrors-1)
                except EOFError:
                    print 'Error reading data'
                    self.keeprunning = False

        self.ClosePort()
        self.EndRawLog()
    
    def Pause(self, pause=None):
        try: 
            
            pass#self.ser.flushInput()
        except:
            pass
        if pause==None:
            self.paused = not self.paused
        else:
            self.paused = pause
    
 
    def Close(self, join=False):
        self.keeprunning = False
        if join: self.join(timeout=3.)
        #if self.baudrate == 115200: self.ser.write('f') # FIX
        try: self.ser.close()
        except: 
            if DEBUG:
                traceback.print_exc()
                #pass
        self.timingMemory.close()
        if self.live and self.rawlog:
            try:
                self.CreateZipFile()
            except:
                traceback.print_exc()
        try: 
            if hasattr(self, 'tmgfile') and hasattr(self.tmgfile, 'close'): self.tmgfile.close();
            if hasattr(self, 'tmgfile'):  del self.tmgfile
            if hasattr(self, 'hegfile'):  self.hegfile.close();  del self.hegfile
            if hasattr(self, 'hegafile'): self.hegafile.close(); del self.hegafile
            if hasattr(self, 'tmgafile'): self.tmgafile.close(); del self.tmgafile
            if hasattr(self, 'zf'):       self.zf.close()      ; del self.zf
            if hasattr(self, 'metadata')  and self.metadata and hasattr(self.metadata, 'close'):
                self.metadata.close()
            self.MaybeUpdateMetadata(unlink=True)
            if DEBUG:
                print "Closed with self.metadata = ", self.metadata
        except: 
            if DEBUG: traceback.print_exc()

    def ClosePort(self):
        try:
            #if DEBUG: print "trying to close serial port"
            self.connected = False
            self.ser.close()
        except:
            pass

class PeanutManager(SerialManager): # CONTAINS PEANUT ENCODINGS
    packet_length_pos = 3
    default_packet_length = 4
    file_extension = 'pea'
    def __init__(self, *args, **kwargs):
        SerialManager.__init__(self, baudrate=38400, *args, **kwargs)
        self.lasttimestamp = time.time()
        self.samplerate = 10.101 # that's roughly what it usually is for peanuts

    def Read(self, *args, **kwargs):
        # This isn't really associated with reading, 
        # but needs to be run periodically
        if self.live and time.time() - self.lasttimestamp > 2.:
            self.lasttimestamp = now = time.time()
            intpart = int(now) % 1000000
            decpart = int((now % 1)*1000000)
            self.write('time %06i.%06i\n' % (intpart, decpart))
        return SerialManager.Read(self, *args, **kwargs)

    def InitSerialPort(self, evt=None):
        SerialManager.InitSerialPort(self, evt=evt)
        if self.live and self.connected:
            try:
                self.ser.write('protocol 3\n')
                self.ser.flush()
            except:
                self.connected = False

    def FindPacketStart(self, pkt):
        try:
            return pkt.index('\xaa\xaa')
        except ValueError:
            return -1
    
    def IsComplete(self, pkt):
        if not pkt[:2] == '\xaa\xaa':
            return False
        try:
            l = ord(pkt[2])
            return len(pkt) >= l+4
        except IndexError:
            pass
        return False

    def GuessPacketLength(self, pkt):
        if len(pkt) < 3:
            return 0
        return ord(pkt[2]) + 4
    
    def CheckChecksum(self, pkt_chr):
        if len(pkt_chr) < 4: 
            if DEBUG: print 'checksum check'
            return False
        chksum = sum(pkt_chr[3:-1])
        chksum = 255 ^ (chksum % 256)
        return chksum == pkt_chr[-1]
        
    def Parse(self, pkt):
        pkt_chr = map(ord, pkt)
        if not self.CheckChecksum(pkt_chr):
            return {}
        results = {}
        i = 3
        while i < len(pkt)-1:
            excode = 0
            code = pkt_chr[i]
            i += 1
            while code == 0x55:
                excode += 1
                i += 1
                code = pkt_chr[i]
            if code > 0x7f:
                length = pkt_chr[i]
                i+=1
            else:
                length = 1
            
            if   code == 0x02:
                results['POOR_SIGNAL']   = struct.unpack('<B',        pkt[i])[0]
            elif code == 0x90:
                results['unfilteredHEG'] = struct.unpack('<i',        pkt[i:i+length])[0]/256.
            elif code == 0x91:
                results['filteredHEG']   = struct.unpack('<i',        pkt[i:i+length])[0]/256.
            elif code == 0x93:
                results['rawdata4']      = struct.unpack('<iiii',     pkt[i:i+length])
            elif code == 0x94:
                results['rawdata6']      = struct.unpack('<iiiiii',   pkt[i:i+length])
            elif code == 0xA0:
                results['sampleNumber']  = struct.unpack('<i',        pkt[i:i+length])[0]
            elif code == 0xB0:
                results['debug0']        = struct.unpack('<i',        pkt[i:i+length])[0]
            elif code == 0xB1:
                results['debug1']        = struct.unpack('<i',        pkt[i:i+length])[0]
            elif code == 0xB2:
                results['debug2']        = struct.unpack('<i',        pkt[i:i+length])[0]
            elif code == 0xB3:
                results['debug3']        = struct.unpack('<i',        pkt[i:i+length])[0]
            elif code == 0xB4:
                results['debug4']        = struct.unpack('<iiiiii',   pkt[i:i+length])
            elif code == 0xB5:
                results['debug4']        = struct.unpack('<iiiiii',   pkt[i:i+length])
            elif code == 0xB6:
                try:
                    results['rawdata27']     = struct.unpack('<B'+'i'*26, pkt[i:i+length])
                except:
                    traceback.print_exc()
                    print length
                    print map(ord, pkt[i:i+length])
            i += length
        return results
        
    def Close(self, *args, **kwargs):
        #try:
        #      self.ser.write('protocol 2\n')
        #except:
        #      pass
        print 'Closing Serial Manager'
        SerialManager.Close(self, *args, **kwargs)
        
class PNBManager(SerialManager):
    default_packet_length = 20
    file_extension = 'pnb'
    def __init__(self, *args, **kwargs):
        SerialManager.__init__(self, baudrate=38400, *args, **kwargs)
        self.is_synced = False
        self.current_frame = ''
        self.current_section = 3
        self.resolution = -1
        self.ALC = 0
    
    def FindPacketStart(self, pkt):
        if self.is_synced:
            return 0
        else:
            for i in range(len(pkt)-19):
                if self.CheckChecksum(pkt[i:]):
                    return i
            return -1
    
    def GuessPacketLength(self, pkt):
        return 20
    
    def IsComplete(self, pkt):
        if len(pkt) >= self.GuessPacketLength(pkt):
            return self.CheckChecksum(pkt)
        
    def SetConfig(self, pkt_chr):
        self.low_bat     = pkt_chr[0]>>5&1
        self.resolution  = pkt_chr[0]>>3&3
        self.sample_rate = pkt_chr[0]&7
        self.SetALC(pkt_chr)
        self.is_synced = True

    def SetALC(self, pkt):
        if len(pkt) >= 6:
            self.ALC = [pkt[5] & 7] # we'll only bother with channel 1 for now

    def CheckChecksum(self, pkt):
        #if len(pkt) >= 20:
        #    sync = ord(pkt[0]>>6) == 0 and ord(pkt[5]>>6) == 1 and ord(pkt[10]>>6) == 2 and ord(pkt[15]>>6) == 3
        #    if not sync: self.is_synced = False
        #    return sync
        for i in range(0, len(pkt), 5):
            if (ord(pkt[i])>>6) != i/5:
                self.is_synced = False
                return False
        return True

    def Parse(self, pkt):
        try:            
            pkt_chr = map(ord, pkt)
            self.SetConfig(pkt_chr)
            
                
            #if not self.CheckChecksum(pkt):
            #    if DEBUG: print "bad packet"
            #    return        
            #if DEBUG: print self.resolution, self.ALC, self.sample_rate
            if not pkt: return
            if self.resolution == 3:
                res  = pkt_chr[1] + pkt_chr[3]/256. - 128.
            elif self.resolution == 2 or self.resolution == 0: # Workaround: Pocket Neurobics devices (at least the pocket A3)
                #self.SetALC(frame)                            # actually don't set the resolution flag
                res = float(pkt_chr[16]) + self.ALC[0] / 8. - 128.
            else:
                res = float(pkt_chr[1]) - 128.
            if (res < 0.):
                res += 256.
            return [res]
        except IndexError:
            return []
            
class HEGduinoManager(SerialManager):
    def __init__(self, *args, **kwargs):
        print 'HEGduino Manager Initialized'
        SerialManager.__init__(self, baudrate=115200, *args, **kwargs)

    def Read():
        return 0

    def Parse():
        return 0

class AutoHEG(threading.Thread):
    def __init__(self, port, baudrate, timeout, callback=None, *args, **kwargs):
        threading.Thread.__init__(self)
        print 'Autodetecting HEG'
        self.port, self.args, self.kwargs, self.callback = port, args, kwargs, callback
        #if hasattr(self.baudrate, '__len__'): # deal with multiple baudrates to test?
        #    baudrate = baudrate[0]
        self.baudrate = baudrate
        self.pnbdevicetype = None
        self.filter = lambda x: x[-1]
        self.mgr = None

        self.use_scoring = True
        self.use_sma = False

        if baudrate != 115200:
            self.detector = ProtocolDetector(port, baudrate, timeout)
            self.detector.start()
    
    def LogRawToFile(self, filename, *args, **kwargs):
        if 'timing' in kwargs: timing = kwargs['timing']
        else: timing = True
        if self.mgr:
            self.mgr.LogRawToFile(filename, timing)
        else:
            self.kwargs['rawlog'] = filename
    def OnData(self, datum):
        print datum
        if not datum and not type(datum) in (int, float):
            return
        
        #self.times.append(time.time())
        
        if type(datum) in (list, tuple):
            if DEBUG: print 'FIXME!', datum
            datum = datum[0] # FIXME!
        if type(datum) in (float, int):
            self.unfilteredhegdata.append(datum)
            self.hegdata.append(self.filter(datum))
            if self.callback:
                return self.callback(self.hegdata[-1], {'unfilteredHEG':datum, 'filteredHEG':self.hegdata[-1]})
            else: return
        elif type(datum) == dict:
            for k,v in datum.items():
                if k in self.data:
                    self.data[k].append(v)
                else:
                    self.data[k] = [v]
                if   k == 'unfilteredHEG':
                    if DEBUG and type(v) in (list, tuple):
                        print 'found a problem here:\t', k, v
                    self.unfilteredhegdata.append(v)
                elif k == 'filteredHEG':
                    self.hegdata.append(v)
            self.dicts.append(datum)
            if self.callback: return self.callback(self.hegdata[-1], datum)     
            else: return       
    
    def OnPNBData(self, datum):
        if not self.pnbdevicetype and hasattr(self, 'hegdata') and len(self.hegdata) > 10:
            #print "Autodetecting device"
            self.pnbdevicetype = self.AutodetectDevice()
            if DEBUG: print "device type is", self.pnbdevicetype
            #print self.resolution
            if self.pnbdevicetype.startswith('pendant'):
                self.SetFilter(linear_FIR_filter_factory(128))
                        
        self.unfilteredhegdata.extend(datum)
        self.hegdata.append(self.filter(self.unfilteredhegdata))
        #print len(self.hegdata), len(self.unfilteredhegdata), type(self.hegdata), type(self.unfilteredhegdata)
        
        #self.times.append(time.time())
        if self.callback: return self.callback(self.hegdata[-1], {'unfilteredHEG':datum, 'filteredHEG':self.hegdata[-len(datum)]})
        else: return
    
    def OnHEGduinoData(self, datum):
        print "RAW: ", datum
        self.heg_raw.append(datum) # currently receiving ratio. See SerialManager.run() and ProtocolDetector.run(). HEGDUINO STRING: ADC,RATIO,POSITION FROM BASELINE. RATIO POSITION TRANSMITS 'WAIT' IF NO RATIO
        length = len(self.heg_raw)

        dy = 0 # Change in Amplitude from Baseline
        sma = 0  # Simple Moving Average
        score = 0 # Scoring = accumulated change in amplitudes from baseline
        if(length > 20):
            for data in itertools.islice(self.heg_raw, length - 21, length - 1): 
                sma += data
            sma = sma / 20
            self.heg_sma.append(sma)
            print "SMA: ", sma
            if self.use_sma:
                self.hegdata.append(sma)
            elif self.use_scoring:
                if not self.hegdata:
                    score = sma # First score is baseline.
                else:
                    dataLength = len(self.hegdata)
                    if sma < self.heg_sma[dataLength - 1] + 0.01 and sma > self.heg_sma[dataLength-1] - 0.01: # if ratio not changing, don't change score. Change tolerance for sensitivity.
                        score = self.hegdata[dataLength - 1]
                    else:
                        dy = sma - self.heg_sma[dataLength - 1]
                        score = self.hegdata[dataLength - 1] + dy * 10

                self.hegdata.append(score)
            else:
                self.hegdata.append(datum)
        return

    def AutodetectDevice(self):
        hegdatalen = len(self.hegdata)
        if hegdatalen < 10:
            return 'unknown'
        
        data = self.hegdata[-30:]
        
        dl = len(data)
        dataave = sum(data)/dl
        if not dataave:
            return 'unknown'
        slope = sum([(data[i]-dataave)*(i-dl/2.) for i in range(dl)])/sum([(i-dl/2)**2 for i in range(dl)])/dl
        residuals = [data[i] - dataave - slope*(i-dl/2.) for i in range(dl)]
        std = sum([r**2 for r in residuals])**.5/dl
        
        diffs = [data[i] - data[i-1] for i in range(1, dl)]
        rmsdiff = sum([(d/dataave)**2 for d in diffs])**.5
        
        if DEBUG: print dl, dataave, slope, rmsdiff, std
        if rmsdiff < .05:
            for datum in data:
                if (datum*8) - int(datum*8):
                    print 'peanut found'
                    return 'peanut'
            return 'pocket'
        elif rmsdiff < .08:
            if '--debug' in sys.argv: print "Warning: couldn't determine if the device is a pendant or a pocket; assuming pocket."
            print 'ambiguous comport'
            return 'ambiguous' # pockets usually produce rmsdiffs around .003
        if dataave < 1.:
            return 'pendant1'
        else:
            return 'pendant2'

    def SetFilter(self, filter):
        self.filter = filter
        for i in range(1, len(self.unfilteredhegdata)):
            self.hegdata[i] = self.filter(self.unfilteredhegdata[:i])


    def run(self, start=True, *args, **kwargs): # Runs after ProtocolDetector.run() returns
        #if DEBUG: print "Starting to run on port", self.port
        callback = None
        mgr = None
        if self.baudrate == 115200:
            print 'Using Protocol: hegduino'
            mgr = HEGduinoManager
            callback = self.OnHEGduinoData
        else:
            self.detector.join()
            print 'Using Protocol: ', self.detector.protocol
            try: 
                mgr = {'hegduino':HEGduinoManager, 'peanut':PeanutManager, 'ascii':SerialManager, 'pnb':PNBManager}[self.detector.protocol] # Runs derived SerialManager
            except KeyError:
                if DEBUG: 
                    print "Couldn't find a manager for protocol %s on port %s", (self.detector.protocol, self.port)
                    print "Data was: ", map(ord, self.detector.data)
                return
            callback = {'hegduino':self.OnHEGduinoData, 'peanut':self.OnData, 'ascii':self.OnData, 'pnb':self.OnPNBData}[self.detector.protocol] # Run this function every time a reading is taken.
        self.mgr = mgr(port=self.port, callback=callback, *self.args, **self.kwargs)
        if self.baudrate != 115200:
            self.mgr.read_str = self.detector.data
        if start: self.mgr.start()
    

    def DoNothing(self, *args, **kwargs):
        return
    
    def __getattr__(self, name):
        if self.mgr:
            return getattr(self.mgr, name)
        else:
            if name[0].isupper():
                return self.DoNothing
            else:
                return None

