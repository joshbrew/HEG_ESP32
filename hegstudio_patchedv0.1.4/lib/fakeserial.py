#!/usr/bin/env python
from __future__ import with_statement

################################################################################
# fakeserial.py
# A lightweight library for imitating a serial port and serial stream using
# a datafile, and optionally a file with timing information.
#
# Copyright 2007-2012, Jonathan Toomim (jtoomim@jtoomim.org)
# This software is available under the terms of the GNU Lesser Public License,
# version 3.0. For a copy of this license, see the file LGPL.txt, or visit 
# http://www.gnu.org/licenses/lgpl-3.0.txt.
#
# Want to adapt this software for your project? Want to make your biofeedback
# hardware supported by this software? Please do! Email me, and I might even
# help.
################################################################################

version = '0.4.0'

import time, random, traceback, os, sys, StringIO
DEBUG = '--debug' in sys.argv

class FakeSerial:
    def __init__(self, name, mode='rb', byterate=10, timeout=1, fast=False):
        """An object that simulates a read-only serial port using data stored
        in the filename referred to by the 'name' variable with timing 
        determined by byterate.  
        """
        self.timeout = timeout
        if hasattr(name, 'read'):
            self.file = name
        else:
            self.file = open(name, mode)
        self.fast = fast
        if hasattr(self.file, 'length'):
            self.length = self.file.length
            self.pos = 0
            self.notell = True
        else:
            self.pos = 0
            self.seek(0,2)
            self.length = self.tell()
            self.seek(0)
        self.byterate = byterate
        self.starttime = time.time()
    def bytes_until_EOF(self):
        return self.length - self.tell()
    def time(self):
        """Returns how long this object has been "reading" data."""
        return time.time() - self.starttime
    def flushInput(self):
        """Does nothing; exists for compatibility with real serial objects."""
        pass
    def inWaiting(self):
        """Check how many bytes can be read by now."""
        if not self.byterate or self.fast:
            return self.bytes_until_EOF()
        position = self.tell()
        target = int(self.byterate*self.time())
        if target > self.length:
            return self.bytes_until_EOF()
        else:
            return target - position
    def readWithoutTimeout(self, size=0):
        """Reads the lesser of size and self.inWaiting() bytes with timeout."""
        if size > self.bytes_until_EOF():
            size = self.bytes_until_EOF()
        #while size > self.inWaiting() & timeout < 1000:
        #    time.sleep(.001)
        #    timeout += 1
        n = self.inWaiting()
        self.pos += min(n, size)
        return self.file.read(min(n, size))
    
    def seek(self, *args, **kwargs):
        return self.file.seek(*args, **kwargs)
    def tell(self, *args, **kwargs):
        if hasattr(self, 'notell'):
            return self.pos
        return self.file.tell(*args, **kwargs)
    def close(self, *args, **kwargs):
        return self.file.close(*args, **kwargs)
        
    def readWithTimeout(self, size=0):
        """Reads the lesser of size and self.inWaiting() bytes with timeout."""
        timeoutstart = time.time()
        if size > self.bytes_until_EOF():
            size = self.bytes_until_EOF()
        while size > self.inWaiting() and time.time() < timeoutstart + self.timeout:
            time.sleep(.001)
        n = self.inWaiting()
        if not size:
            self.pos += n
            return self.file.read(n)
        self.pos += min(n, size)
        return self.file.read(min(n, size))

    read = readWithTimeout


class FakeSerialTiming(FakeSerial):
    def __init__(self, name, mode='rb', timingfile=None, byterate=10, timeout=1, fast=False):
        """An object that simulates a read-only serial port using data stored in
        the filename referred to by the 'name' variable with timing data
        stored in timingfile.  If no timingfile argument is given, then 
        name + '.timing' is used.  Attempts to fall back to byterate-based
        timing if no such file exists, but this hasn't been tested.
        """
        if timingfile == None:
            if os.path.exists(name+'.tmg'):
                timingfile = name + '.tmg'
            elif os.path.exists(name[:-4] + '.tmg'):
                timingfile = name[:-4] + '.tmg'
        try:
            if timingfile:
                try:
                    tf = timingfile
                    if not hasattr(timingfile, 'read'):
                        tf = open(timingfile, 'rb')
                    self.timing = [(lambda x: [float(x[0]), int(x[1])])(line.split(',')) for line in tf]
                    if self.timing:
                        if self.timing[0][1] > 100:
                            total_bytes = sum([t[1] for t in self.timing[1:]])
                            byterate = total_bytes / (self.timing[-1][0] - self.timing[1][0])
                            self.timing.insert(0, [self.timing[0][0] - self.timing[0][1]/byterate, 1])
                        t0 = self.timing[0][0]
                        for item in self.timing:
                            if not item[1]: self.timing.remove(item)
                        for item in self.timing:
                            item[0] -= t0
                finally:
                    tf.close()
                self.waiting = 0
                self.use_timing = True
                self.lastline = []
            else:
                self.use_timing = False
        except IOError:
            
            #print "Couldn't open file %s." % timingfile
            self.use_timing = False
        FakeSerial.__init__(self, name, mode, byterate, timeout, fast)
    def inWaiting(self):
        """Check how many bytes can be read by now."""
        if self.fast:
            return self.bytes_until_EOF()
        if not self.use_timing: return FakeSerial.inWaiting(self)
        i = 0
        target = 0
        while self.timing and self.timing[i][0] < self.time():
            target += self.timing[i][1]
            i += 1
            if i >= len(self.timing):
                raise EOFError#self.waiting = self.bytes_until_EOF()
        self.waiting = target - self.tell() + \
                       int(self.timing[i][1] * 
                           (self.time() - self.timing[i-1][0]) / 
                           (self.timing[i][0] - self.timing[i-1][0]))
        return self.waiting 
    def inWaiting2(self):
        if self.fast:           return self.bytes_until_EOF()
        if not self.use_timing: return FakeSerial.inWaiting(self)
#        done = sum([bytes for timestamp, bytes
        
        
    def getTime(self):
        """Return a timestamp for the current file position."""
        if not self.use_timing: 
            return self.tell() / self.byterate
        pos = self.tell()
        i = 0
        try:
            cur = self.timing[0][1]
        except IndexError:
            #print "IndexError"
            return 0.
        while i < len(self.timing)-1 and cur < pos:
            i += 1
            cur += self.timing[i][1]
        t1, t2 = self.timing[i-1][0], self.timing[i][0]
        if i == 0: t1 = 0
        c1, c2 = cur-self.timing[i][1], cur
        
        x = float(pos-c1)/float(c2-c1)
        #print x, i, c1, c2, t1, t2, x*(t2-t1) + t1
        #if len(self.timing) == 2: return t2
        return x*(t2-t1) + t1
        
    def read(self, size=1):
        """Reads the lesser of size and self.inWaiting() bytes without timeout."""
        if not self.use_timing: return FakeSerial.read(self, size)
        bytes = FakeSerial.read(self, size)
        self.waiting -= len(bytes)
        return bytes        




SerialDumpers = []
for fileType in (file, StringIO.StringIO):
    
    class SerTim(fileType):
        def __init__(self, name, mode='wb', timingfile=None, resolution=None, 
                     starttime=None, prependdata='', prependtiming=''):
            """An object that logs data for simulating a read-only serial port.
            Serial data are stored in the filename referred to by the 'name' 
            variable with timing data stored in timingfile.  If no timingfile 
            argument is given, then name + '.timing' is used.  Attempts to fall 
            back to byterate-based timing if no such file exists, but this hasn't 
            been tested.
            """
            self.resolution = resolution
            if starttime: self.starttime = starttime
            else:         self.starttime = time.time()
            self.laststamp = time.time()
            self.bytestostamp = 0
            self.fileType = fileType
            self.rename(name, timingfile, mode)
            if self.fileType == StringIO.StringIO:
                self.fileType.__init__(self)
            else:
                self.fileType.__init__(self, self.filename, mode)
            if prependtiming:
                self.timingfile.write(prependtiming)
            if prependdata:
                self.fileType.write(self, prependdata)
            self.logtime()
            
        def time(self):
            """Returns how long this object has been writing data."""
            return time.time() - self.starttime
        
        def write(self, s):
            if self.resolution == None:
                resolution = 1.
            else:
                resolution = self.resolution
            self.bytestostamp += len(s)
            if time.time() - self.laststamp > resolution:
                self.logtime()
            if not self.fileType == StringIO.StringIO:
                self.fileType.write(self, s)
            
        def logtime(self):
            self.laststamp = time.time()
            self.timingfile.write("%s,%s\n" % (repr(self.laststamp), repr(self.bytestostamp)))
            self.bytestostamp = 0
            if self.resolution == None: self.resolution = 1.
            
        def writelines(self, sequence_of_strings):
            raise NotImplementedError
        
        def flush(self):
            self.logtime()
            self.timingfile.flush()
            self.fileType.flush(self)
            
        def close(self):
            oldres = self.resolution
            self.resolution = 0.0
            self.write('')
            self.resolution = oldres
            self.timingfile.close()
            self.fileType.close(self)
        def rename(self, newname, timingfile=None, mode='wb'):
            if '--debug' in sys.argv: print 'newname: ', newname
            if timingfile == None and not newname == None:
                if newname[-4] == '.': timingfile = newname[:-4] + '.tmg'
                else:                  timingfile = newname      + '.tmg'

            if hasattr(self, 'filename'):
                if '--debug' in sys.argv:
                    print 'self.filename: ' + `self.filename`
                    print self.__dict__
                self.close()
                os.rename(self.filename, newname)
                os.rename(self.timingfilename, timingfile)
                mode = mode.replace('w', 'a')
            if self.fileType == StringIO.StringIO:
                self.timingfile = self.fileType()
            else:
                self.timingfile = self.fileType(timingfile, mode)
            self.filename = newname
            self.timingfilename = timingfile
            
            
    SerialDumpers.append(SerTim)
    del SerTim
    
DumpSerialTiming     = SerialDumpers[0]
RememberSerialTiming = SerialDumpers[1]


class SerialTiming:
    def __init__(self, name='', mode='wb', timingfile=None, resolution=None, 
                 starttime=None, prependdata='', prependtiming=''):
        """An object that logs data for simulating a read-only serial port.
        Serial data are stored in the filename referred to by the 'name' 
        variable with timing data stored in timingfile.  If no timingfile 
        argument is given, then name + '.timing' is used.  Attempts to fall 
        back to byterate-based timing if no such file exists, but this hasn't 
        been tested.
        """
        self.resolution = resolution
        if starttime: self.starttime = starttime
        else:         self.starttime = time.time()
        self.laststamp = time.time()
        self.bytestostamp = 0
        self.rename(name, timingfile, mode)
        if name:
            self.datafile = open(name, mode)
        else:
            self.datafile = StringIO.StringIO()
        if prependtiming:
            self.timingfile.write(prependtiming)
        if prependdata:
            self.datafile.write(prependdata)
        self.logtime()
        
    def time(self):
        """Returns how long this object has been writing data."""
        return time.time() - self.starttime
    
    def write(self, s):
        if self.resolution == None:
            resolution = 1.
        else:
            resolution = self.resolution
        self.bytestostamp += len(s)
        if time.time() - self.laststamp > resolution:
            self.logtime()
        self.datafile.write(s)
        
    def logtime(self):
        self.laststamp = time.time()
        self.timingfile.write("%s,%s\n" % (repr(self.laststamp), repr(self.bytestostamp)))
        self.bytestostamp = 0
        if self.resolution == None: self.resolution = 1.
        
    def writelines(self, sequence_of_strings):
        raise NotImplementedError
    
    def flush(self):
        self.logtime()
        self.timingfile.flush()
        self.datafile.flush()
        
    def close(self):
        oldres = self.resolution
        self.resolution = 0.0
        self.write('')
        self.resolution = oldres
        self.timingfile.close()
        self.datafile.close()
        
    def rename(self, newname, timingfile=None, mode='wb'):
        #if '--debug' in sys.argv: print u'newname: ', newname
        if timingfile == None and not newname == None:
            #newname = unicode(newname).encode('utf-8')
            if newname[-4] == '.':
                 timingfile = newname
                 timingfile = timingfile[:-4]
                 timingfile = u''.join([timingfile, u'.tmg'])
            else:                 
                 timingfile = newname.encode('utf-8')      + u'.tmg'

        if hasattr(self, 'filename') and self.filename:
            if '--debug' in sys.argv:
                print 'self.filename: ' + `self.filename`
                print self.__dict__
            self.datafile.close()
            if newname:
                os.rename(self.filename, newname)
                os.rename(self.timingfilename, timingfile)
            mode = mode.replace('w', 'a')
        if not newname:
            self.timingfile = StringIO.StringIO()
        else:
            self.timingfile = open(timingfile, mode)
        self.filename = newname
        self.timingfilename = timingfile