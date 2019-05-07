#!/usr/bin/env python

################################################################################
# HEGDiag
# An application for diagnostics for hemoencephalography systems
# using the nirHEG Peanut and relatives.
# 
# Copyright 2011-2016, Jonathan Toomim (jtoomim@jtoomim.org)
# This software is available under the terms of the GNU Lesser Public License,
# version 3.0. For a copy of this license, see the file LGPL.txt, or visit 
# http://www.gnu.org/licenses/lgpl-3.0.txt.
#
# Want to adapt this software for your project? Want to make your biofeedback
# hardware supported by this software? Please do! Email me, and I might even
# help.
################################################################################

__VERSION__ = version = '0.4.8'

import serial, wx, numpy, traceback, threading, time, sys, struct, copy, os, random
import lib.plot
#import wx.lib.plot
wx.lib.plot = lib.plot

USE_PARSERS = True
if USE_PARSERS: from lib import parsers

DEBUG = '--debug' in sys.argv
N2 = '--n2' in sys.argv

if DEBUG: import wx.py

gain = 1.
fast = 0

if sys.platform == 'darwin':
    sessionsdir = os.path.expanduser('~/Documents/HEG sessions')
else:
    if sys.platform == 'win32':
        import _winreg as winreg

        path = 'Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders'
        try:
            key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, path)
            sessionsdir = os.path.join(winreg.QueryValueEx(key, 'Personal')[0], 'HEG Sessions')
        except WindowsError:
            if DEBUG:
                print "Couldn't find 'My Documents' (or equivalent) folder. Falling back to hardcoded path."
            sessionsdir = os.path.join(os.path.expanduser('~/Documents/HEG sessions'))
heg_wildcard = 'HEG files (*.heg;*.pnb;*.pea;*.heg.zip)|*.heg;*.pnb;*.pea;*.heg.zip'

def ToVolts(val):
    #if val & 1<<31:
    #    val = -(val ^ ((1<<32)-1))
    VREF = 1200.
    val = 1./gain*VREF*val/float(1<<23)
    return val
 
VOLTIFY = True

default_colors = ['red', 'orange', 'green', 'blue', 'purple', 'black', 'gray']*100
signal_choices = []
peanut_kernels = ['filteredHEG',
           'IR',#'rawdata6_1-((rawdata6_0+rawdata6_2)/2)',
           'RED',#'(air1+air2)/2',#'(rawdata6_0 + rawdata6_2)/2', 
           #'8.0 * ( log(IR*.5) - EHbI/EHbR * log(RED)) / (EHbOR*EHbI/EHbR - EHbOI)',
           #'8.0 * ( log(IR*.5) - EHbOI/EHbOR * log(RED)) / (EHbR*EHbOI/EHbOR - EHbI)',           
           'ared',#'rawdata6_4 - (rawdata6_3 + rawdata6_5)/2',
           'air',#'(ared1+ared2)/2',#'(rawdata6_3 + rawdata6_5)/2',
           'rawdata6']
           #'(( log(IR*.5) - EHbOI/EHbOR * log(RED)) / (EHbR*EHbOI/EHbOR - EHbI)) / (( log(IR*.5) - EHbI/EHbR * log(RED)) / (EHbOR*EHbI/EHbR - EHbOI))']

neuro2_kernels = ['filteredHEG',
           'IR',#'rawdata6_1-((rawdata6_0+rawdata6_2)/2)',
           'RED',#'(air1+air2)/2',#'(rawdata6_0 + rawdata6_2)/2', 
           #'8.0 * ( log(IR*.5) - EHbI/EHbR * log(RED)) / (EHbOR*EHbI/EHbR - EHbOI)',
           #'8.0 * ( log(IR*.5) - EHbOI/EHbOR * log(RED)) / (EHbR*EHbOI/EHbOR - EHbI)',           
           'ared',#'rawdata6_4 - (rawdata6_3 + rawdata6_5)/2',
           'air',#'(ared1+ared2)/2',#'(rawdata6_3 + rawdata6_5)/2',
           'rawdata6']
           #'(( log(IR*.5) - EHbOI/EHbOR * log(RED)) / (EHbR*EHbOI/EHbOR - EHbI)) / (( log(IR*.5) - EHbI/EHbR * log(RED)) / (EHbOR*EHbI/EHbR - EHbOI))']

if '--neuro2' in sys.argv:
    kernels = neuro2_kernels
    N2 = True
else:
    kernels = peanut_kernels


if N2:
    kernels = ['rawdata6',
               '(-rawdata6_4 + (rawdata6_3*2 + rawdata6_0)/3.) / (-rawdata6_5 + rawdata6_3) ',
               '(-rawdata6_2 + (rawdata6_1 + rawdata6_3)/2.) / -(rawdata6_5 - rawdata6_3)',
               '-rawdata6_4 + (rawdata6_3*2 + rawdata6_0)/3.',
               '-(rawdata6_5 - rawdata6_3)',
               '-rawdata6_2 + (rawdata6_1 + rawdata6_3)/2.']

    kernels = ['rawdata27',
               '(-rawdata6_4 + (rawdata6_3*2 + rawdata6_0)/3.) / (-rawdata6_5 + rawdata6_3) ',
               '(-rawdata6_2 + (rawdata6_1 + rawdata6_3)/2.) / -(rawdata6_5 - rawdata6_3)',
               '-rawdata6_4 + (rawdata6_3*2 + rawdata6_0)/3.',
               '-(rawdata6_5 - rawdata6_3)',
               '-rawdata6_2 + (rawdata6_1 + rawdata6_3)/2.']
    kernels = ['l3', # 660
               'l2' , # 
               'l5', # ??? nm
               'l6',
               'l5 / l6',
               'l3 / l6',]



class PlotCanvas(wx.lib.plot.PlotCanvas):
    def __init__(self, parent, color=['red', 'orange', 'green', 'blue', 'purple', 'black', 'gray'], size=(950,200), asVolts=True):
        wx.lib.plot.PlotCanvas.__init__(self, parent)
        self.color = color
        self.SetMinSize((200, 50))
        self.SetInitialSize(size)
        self.asVolts = asVolts
        self.SetFontSizeTitle(10)

        
        
    def Update(self, data, times=None, colors=None, n=None, xLabel='', yLabel='', title='', legend=['']*20):
        minpersample = 1.#.008192 / 60. * 4. # minutes per sample; ~= 30.5 hz
        #data = data[:]
        
        if not len(data): return
        for i in range(len(legend)):
            if legend[i] == 'rawdata6':
                del legend[i]
                for n in range(5,-1,-1):
                    legend.insert(i, 'rawdata6_%i'%n)
        for i in range(len(legend)):
            if legend[i] == 'debug4':
                del legend[i]
                for n in range(5,-1,-1):
                    legend.insert(i, 'debug4_%i'%n)
            elif legend[i] == 'rawdata4':
                del legend[i]
                for n in range(3,-1,-1):
                    legend.insert(i, 'rawdata4_%i'%n)
        lines = []
        if not colors:
            if type(self.color) == list:
                colors = self.color
            else:
                colors = [self.color]
        if len(colors) < 20:
            colors *= 20
        if len(colors) < len(data):
            colors *= 20

        if type(data[0]) in (float, int):
            data = [data]
        
        for i in range(len(data)):
            if type(data[i]) == numpy.ndarray and len(data[i].shape) > 1:
                for j in range(data[i].shape[1]):
                    m = min(data[i].shape[0], len(times[i]))
                    if m<2: return
                    data1 = numpy.zeros((m, 2))
                    data1[:,0] = numpy.array(times[i][-m:])
                    data1[:,1] = data[i][-m:,j]
                    lines.append(wx.lib.plot.PolyLine(data1, colour=colors[i+j]))#, legend=legend[i+j]))
            else:
                if n: m = min(n, len(data[i])-4)
                else: 
                    if len(times):
                        m = min(len(data[i]), len(times[i]))
                    else:
                        m = len(data[i])
                if m < 2:
                    return
                data1 = numpy.zeros((m, 2))
                if len(times):
                    data1[:,0] = numpy.array(times[i][-m:])
                else:
                    data1[:,0] = numpy.arange(len(data[i][-m:])) * minpersample
                try:
                    data1[:,1] = numpy.array(data[i][-m:])
                except:
                    traceback.print_exc()
                    print data[i][-m:], i, m
                    print data
                    return
                try:
                    lines.append(wx.lib.plot.PolyLine(data1, colour=colors[i], legend=legend[i]))
                except:
                    legend = (legend[0] + "_").join(map(str, range(data1.shape[1])))
                    lines.append(wx.lib.plot.PolyLine(data1, colour=colors[i], legend=legend[i]))

                
        
#        else:
#            data1 = numpy.zeros((len(data), 2))
#            data1[:,1] = data
#            if self.asVolts:
#                data1[:,1] = ToVolts(data1[:,1])
#            if not times or not len(times) >= len(data):
#                data1[:,0] = numpy.arange(len(data)) * minpersample
#            else:
#                t0 = times[-len(data)]
#                t1 = times[-1]
#                try:  data1[:,0] = numpy.arange(t0, t1, (t1-t0)/len(data))[-len(data):]#times[-len(data):]
#                except:  return
#        
#            lines.append(wx.lib.plot.PolyLine(data1, colour=self.color))
        if not self.GetEnableLegend() == (len(lines) > 1):
            self.SetEnableLegend(len(lines) > 1)
        graph = wx.lib.plot.PlotGraphics(lines, xLabel=xLabel, yLabel=yLabel, title=title)
        try: self.Draw(graph)
        except wx.PyDeadObjectError: print "dead object access"

class ConfigurablePlot(PlotCanvas):
    def __init__(self, parent, signals=[], *args, **kwargs):
        PlotCanvas.__init__(self, parent, *args, **kwargs)
        self.parent = parent
        if sys.platform == 'darwin':
            self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightClick)
        self.Bind(wx.EVT_CONTEXT_MENU, self.OnRightClick)
        self.signals = signals
        
    def OnRightClick(self, evt=None):
        def MakeHandler(signal):
            return lambda x: self.OnSignalChange(x, signal=signal)
            
        menulist = []
        for signal in signal_choices:
            menulist.append([signal, MakeHandler(signal)])
            if signal.startswith('rawdata'):
                for i in range(int(signal[-1])):
                    fullsig = '%s_%i' % (signal, i)
                    menulist.append([fullsig, MakeHandler(fullsig)])
                    
        newmenu = wx.Menu()
        for item in menulist:
            newmenuitem = wx.MenuItem(parentMenu=newmenu, id=-1, text=item[0])
            self.Bind(wx.EVT_MENU, item[1], newmenuitem)
            newmenu.AppendItem(newmenuitem)
        self.PopupMenu(newmenu, self.ScreenToClient(evt.GetPosition()))
    
    def OnSignalChange(self, evt=None, signal=None):
        if signal in self.signals:
            self.signals.remove(signal)
        else:
            self.signals.append(signal)
        
    def Update(self, data, times=None, datadict=None, colors=None, *args, **kwargs):
        legend = []
        if datadict and self.signals:
            data = []
            times = []

            #print "starting with data ", data, self.signals
            for signal in self.signals:
                if signal.startswith('rawdata'):
                    if '_' in signal:
                        legend.append(signal)
                        #data.append(datadict[signal.split('_')[0]][int(signal.split('_')[1])])
                        data.append(datadict[int(signal.split('_')[1])])
                        #print "appended " , datadict[int(signal.split('_')[1])]
                        times.append(datadict['rawtimes'])
                    else:
                        for i in range(int(signal.split('_')[1])):
                            legend.append(signal + '_%i'%i)
                            data.append(datadict[i])
                            times.append(datadict['rawtimes'])
                else:
                    try:
                        times.append(datadict['rawtimes'])
                        legend.append(signal)
                        if 1:#signal.startswith('kernel'):
                            data.append(datadict[signal])
                        else:
                            data.append(kernels[self.signals.index(signal)])

                    except KeyError as key:
                        #if key.args[0].startswith('kernel'):
                        #    pass
                        #else:
                            traceback.print_exc()
            #print "ended with data ", data
            #colors = ['red', 'orange', 'green', 'blue', 'purple', 'black'][:len(data)]
        signals = self.signals[:]
        for i in range(len(signals)):
            if signals[i].startswith('kernel'):
                signals[i] = kernels[int(signals[i][-1])]
        if not signals:
            signals = ['']*6
        times = numpy.array(times)
        try:
            times -= times[0][0]
        except:
            traceback.print_exc()
            print "times.shape = ", times.shape
            print "datadict['rawtimes'] =", datadict['rawtimes']
            print signal
        times = list(times)
        PlotCanvas.Update(self, data, times=times, legend=signals, xLabel='', colors=colors, title=', '.join(signals), *args, **kwargs)


class MyApp(wx.PySimpleApp):
    def OnInit(self, evt=None):
        self.user = 'HEGDiag'
        self.mainframe = mainframe = wx.mainframe = wx.Frame(parent=None, title="HEGDiag", size=(1100, 780))
        if DEBUG:
            mainframe.SetSize((1100, 1400))
        mainpanel = self.mainpanel = wx.ScrolledWindow(parent=mainframe)
        mainpanel.SetScrollRate(10,10)
        mainpanel.sizer = wx.GridBagSizer()
        mainpanel.plot1 = ConfigurablePlot(parent=mainpanel, signals=['kernel0'])
        mainpanel.plot2 = ConfigurablePlot(parent=mainpanel, color=['brown']  + default_colors, signals=['kernel1'])
        mainpanel.plot3 = ConfigurablePlot(parent=mainpanel, color=['purple'] + default_colors, signals=['kernel2'])
        mainpanel.plot4 = ConfigurablePlot(parent=mainpanel, color=['black']  + default_colors, signals=['kernel3'])
        mainpanel.plot5 = ConfigurablePlot(parent=mainpanel, color=['blue']   + default_colors, signals=['kernel4'])
        mainpanel.plot6 = ConfigurablePlot(parent=mainpanel, color=['green']  + default_colors, signals=['kernel5'])
        self.plots = [mainpanel.plot1,
                      mainpanel.plot2,
                      mainpanel.plot3,
                      mainpanel.plot4,
                      mainpanel.plot5,
                      mainpanel.plot6]
        

                      
        mainpanel.sizer.Add(mainpanel.plot1, (1, 0), (4,1))
        mainpanel.sizer.Add(mainpanel.plot2, (6, 0), (4,1))
        mainpanel.sizer.Add(mainpanel.plot3, (11,0), (4,1))
        mainpanel.sizer.Add(mainpanel.plot4, (16,0), (4,1))
        mainpanel.sizer.Add(mainpanel.plot5, (21,0), (4,1))
        mainpanel.sizer.Add(mainpanel.plot6, (26,0), (4,1))
        
        #slider = wx.Slider(mainpanel)
        #mainpanel.sizer.Add(slider, (31, 0), (4, 1))
        
        mainpanel.connectbtn = wx.Button(parent=mainpanel, label="Disconnect")
        mainpanel.connectbtn.Bind(wx.EVT_BUTTON, self.ToggleConnection)
        mainpanel.sizer.Add(mainpanel.connectbtn, (0,2), (1,1))

        mainpanel.loadbtn = wx.Button(parent=mainpanel, label="Load file")
        mainpanel.loadbtn.Bind(wx.EVT_BUTTON, self.OpenSession)
        mainpanel.sizer.Add(mainpanel.loadbtn, (1,2), (1,1))
        
        mainpanel.savebtn = wx.Button(parent=mainpanel, label="Save file")
        mainpanel.savebtn.Bind(wx.EVT_BUTTON, self.SaveSession)
        mainpanel.sizer.Add(mainpanel.savebtn, (2,2), (1,1))
        
        mainpanel.clearbtn = wx.Button(parent=mainpanel, label="Clear")
        mainpanel.clearbtn.Bind(wx.EVT_BUTTON, self.Clear)
        mainpanel.sizer.Add(mainpanel.clearbtn, (3,2), (1,1))

        mainpanel.peanutctrl = wx.TextCtrl(parent=mainpanel, id=-1, style=wx.TE_PROCESS_ENTER)
        mainpanel.peanutctrl.Bind(wx.EVT_TEXT_ENTER, self.SendPeanutMessage)
        mainpanel.sizer.Add(mainpanel.peanutctrl, (4,2), (1,1))
        
        if DEBUG:
            mainpanel.crustbtn = wx.Button(parent=mainpanel, label="PyCrust")
            mainpanel.crustbtn.Bind(wx.EVT_BUTTON, self.OpenCrust)
            mainpanel.sizer.Add(mainpanel.crustbtn, (5,2), (1,1))
            mainpanel.fillbtn = wx.Button(parent=mainpanel, label="PyFilling")
            mainpanel.fillbtn.Bind(wx.EVT_BUTTON, self.OpenFilling)
            mainpanel.sizer.Add(mainpanel.fillbtn, (6,2), (1,1))
            
            self.crustframes = []
            self.fillings = []
            

        def MakeHandler(n):
            return lambda evt: self.SetKernel(evt, n)
        num_kernels = 6
        self.kerntctls = []
        self.kernels = kernels
        self.kernelexceptions = []
        
        for i in range(num_kernels):
            self.kerntctls.append(wx.TextCtrl(parent=mainpanel, id=-1, value=kernels[i], size=(900,20), style=wx.TE_PROCESS_ENTER|wx.TE_CENTER))
            self.kerntctls[-1].Bind(wx.EVT_TEXT_ENTER, MakeHandler(i))
            self.kernels.append('filteredHEG')
            mainpanel.sizer.Add(self.kerntctls[-1], (5*i,0), (1,1))
            signal_choices.append('kernel%i'%i)
            
        if sys.platform == 'darwin':
            for plot, kctl in zip(self.plots, self.kerntctls):
                kctl.Bind(wx.EVT_CONTEXT_MENU, plot.OnRightClick)
                
        mainpanel.SetSizer(mainpanel.sizer)
        mainpanel.sizer.Layout()
        
        mainframe.Bind(wx.EVT_CLOSE, self.OnClose)
        
        if map(int, wx.version().split(' ')[0].split('.')) < [2, 8, 10]:
            oldwxplot = True
        else:
            oldwxplot = False
        for plot in (mainpanel.plot1, mainpanel.plot2, mainpanel.plot3, mainpanel.plot4, mainpanel.plot5, mainpanel.plot6):
            if not oldwxplot:
                plot.SetForegroundColour('black')
                plot.SetGridColour('black')
                #plot.SetYSpec('auto')
                plot.SetYSpec(5)
                plot.SetEnableAntiAliasing(True)
                plot.SetEnableHiRes(True)
                plot.SetXSpec('min')
            else:
                plot.SetYSpec('auto')

                
        self.Clear()
        
        self.Connect()
        
        self.timer = wx.Timer()
        self.timer.Bind(wx.EVT_TIMER, self.UpdatePlots)
        self.timer.Start(100)
        
        mainframe.Show(True)
        return True
    
    def SendPeanutMessage(self, evt=None):
        global gain
        global fast
        tc = self.mainpanel.peanutctrl
        text = tc.GetValue()
        if not text.endswith('\n'):
            text = text + '\n'
        if hasattr(self, 'serialManager') and self.serialManager.connected and not text.startswith('fast'):
            self.serialManager.write(text.encode('ascii'))
            #if text.startswith('gain '):
            #    try:
            #        gain = 2.**(int(text.split(' ')[1])+1)
            #    except:
            #        traceback.print_exc()
            #        print text
            if text.startswith('reset'):
                try:
                    print "Port is ", self.serialManager.port
                    self.Disconnect()
                except:
                    pass
        elif text.startswith('fast'):
            try:
                fast = int(text.split()[1])
            except:
                traceback.print_exc()
    
    def SetKernel(self, evt, n):
        self.kernels[n] = str(self.kerntctls[n].GetValue())
        self.UpdatePlots()
        
    def RunKernels(self):
        #l = {}
        l = copy.copy(numpy.__dict__)
        for k in self.data:
            if type(k) in (unicode, str) and k.startswith('kernel'):
                continue
            if type(k) in (unicode, str) and not k.startswith('rawdata'):
                try:
                    l[k] = numpy.array(self.data[k])
                except ValueError:
                    if DEBUG:
                        traceback.print_exc()

                        print '\n'*3
                        print k
                        print '\n'*3
                        #print l.keys()
                        #print '\n'*3
                        print self.data[k]
                        print '\n'*3
                        #print numpy.array(self.data[k])
                        #app.OnClose()
                        #sys.exit(1)
#            elif type(k) in (unicode, str) and k.startswith('rawdata'):
#                s = int(k[-1])
#                for n in range(s):
#                    l["%s_%i"%(k,n)] = numpy.array(self.data[n+1])
#                l[k] = numpy.array(self.data['all'])
#                l[k] = numpy.reshape(l[k], (len(l[k])/s,s), 'C')
            elif type(k) in (unicode, str) and k.startswith('rawdata'):
                s = int(k.split('data')[1])
                #l[k] = numpy.array(self.data['all'])
                l[k] = numpy.array(self.data[k])
                l[k] = numpy.reshape(l[k], (l[k].size/s,s), 'C')
                
                    #l[k] = l[k][:12*(len(l[k])/12)]
                    #try:
                    #    l[k] = numpy.reshape(l[k], (len(l[k])/s,s), 'C')   
                    #except:
                    #    print l[k].shape, len(l[k])/s, s
                for n in range(s):
                    l["%s_%i"%(k,n)] = numpy.ravel(l[k][:,n])
                    l["rd%i"%(n)]    = l["%s_%i"%(k,n)]
                if 'rd12' in l:
                    for i in range(1,7):
                        l['l%i'%i] = l['rd%i'%(2*i-1)]/2. + l['rd%i'%(2*i+1)]/2. - l['rd%i'%(2*i)] 

        if not 'rawdata6_0' in l:
            try:
                for i in range(6):
                    l['rawdata6_%i' % i] = l['rawdata27_%i' % i]
            except:
                exc = traceback.format_exc()
                if not exc in self.kernelexceptions:
                    self.kernelexceptions.append(exc)
                    print exc
                    if len(self.kernelexceptions) > 10:
                        print "deleting an exception"
                        del self.kernelexceptions[0]

        try:
            for k in l:
                if k.startswith('rawdata') and '_' in k:
                    n = int(k.split('_')[1])
                    l['rd%i'%n] = l[k]
            l['red']   = l['rawdata6_4']
            l['ir']    = l['rawdata6_1']
            l['air1']  = l['rawdata6_0']
            l['air2']  = l['rawdata6_2']
            l['ared1'] = l['rawdata6_3']
            l['ared2'] = l['rawdata6_5']
            l['ared']  = (l['ared1'] + l['ared2'])/2
            l['RED']   = l['red'] - l['ared']
            l['air']   = (l['air1']  + l['air2'])/2
            l['IR' ]   = l['ir']  - l['air']
            l['EHbR']  = 3226.
            l['EHbOR'] = 320.
            l['EHbI']  = 691.
            l['EHbOI'] = 1092.
        except KeyError:
            pass
        for n in range(len(self.kernels)):
            kern = self.kernels[n]
            try:
                self.data['kernel%i'%n] = eval(kern, globals(), l)
            except:
                exc = traceback.format_exc()
                if not exc in self.kernelexceptions:
                    self.kernelexceptions.append(exc)
                    print exc
                    if len(self.kernelexceptions) > 10:
                        print "deleting an exception"
                        del self.kernelexceptions[0]       
                        
    def Clear(self, evt=None):
        self.data  = {'all':[], 'times':[]}
        self.times = {'all':[], 'times':[]}
        for i in range(27):
            self.data[i] = []
            self.times[i] = []
        for i in range(len(self.kernels)):
            self.data['kernel%i'%i] = []
            self.times['kernel%i'%i] = []
            
        self.starttime = time.time()

    def OnNewData(self, data, datadict={}):
        try:
            if datadict:
                for k in datadict.keys():
                    if not k in signal_choices:
                        signal_choices.append(k)
                if DEBUG: print datadict
                now = time.time()
                #VREF = 1200.
                # = 2.*VREF*val/float(2<<24)
                for k,v in datadict.items():
                    if not k in self.data:
                        self.data[k] = [v]
                    else:
                        self.data[k].append(v)
                self.data['times'].append(now - self.starttime)
                #self.data
                
                HEG = datadict['unfilteredHEG']
                self.data[7].append(HEG)
                if VOLTIFY:
                    voltify = ToVolts
                else:
                    voltify = lambda x: x
                if 'rawdata27' in datadict:
                    #if not 'rawdata6' in datadict: # fixme
                    #    datadict['rawdata6'] = datadict['rawdata27'][1:7]
                    for i in range(27):
                        self.times[i].append(now-self.starttime)
                        self.data[i].append(voltify(datadict['rawdata27'][i]))
                        self.times['all'].append(now-self.starttime)
                        self.data['all'].append(voltify(datadict['rawdata27'][i]))
                
                if 'rawdata6' in datadict:
                    for i in range(6):
                        self.times[i].append(now-self.starttime)
                        self.data[i].append(voltify(datadict['rawdata6'][i]))
                        self.data['all'].append(voltify(datadict['rawdata6'][i]))
                        self.times['all'].append(now-self.starttime)
                elif 'rawdata4' in datadict:
                    for i in range(4):
                        self.times[i].append(now-self.starttime)
                        self.data['all'].append(voltify(datadict['rawdata4'][i]))
                        self.data[i].append(voltify(datadict['rawdata4'][i]))
                        self.times['all'].append(now-self.starttime)
                        
                    
                    
#                     elif 'rawdata6' in datadict:
#                        rd6 = datadict['rawdata6']
#                        fourtet = [rd6[1], (rd6[0]+rd6[2])/2, rd6[4], (rd6[3]+rd6[5])/2]
#                        for i in range(4):
#                            self.times['all'].append(now-self.starttime)
#                            self.times[i+1].append(now-self.starttime)
#                            self.data['all'].append(fourtet[i])
#                            self.data[i+1].append(fourtet[i])
                if not len(self.times[1]) % 100:
                    try:
                        print "After %i samples, average sampling rate is %f Hz (%f seconds)" % \
                            (len(self.times[1]), float(len(self.times[1])-21)/(self.times[1][-1]-self.times[1][20]), self.times[1][-1])
                    except: pass
            
            if self.waterfall:
                self.UpdateWaterfall()

        except:
            traceback.print_exc()
    def UpdatePlots(self, evt=None):
        self.RunKernels()
        try:
            if not hasattr(self, 'serialManager'):# or not self.serialManager.connected:
                return
            displayTime = 4. # seconds
            n = 1
            while n < len(self.times[3])-1 and self.times[3][-1] - self.times[3][-n-1] < displayTime:
                n += 1
#                print n
#            print n, len(self.times[4]), 
            #n = 200
            #n = min(n, len(self.data[4]))
            
            if 0:
                red  = numpy.array(self.data[3][-n:])
                ared = numpy.array(self.data[4][-n:])
                ir   = numpy.array(self.data[1][-n:])
                air  = numpy.array(self.data[2][-n:])
                
                mir  = numpy.mean(ir)
                mair = numpy.mean(air)
                mirair = (mir + mair)/2.
                
                #y = (ir-mirair)/(air-mirair)
                m = 3
                dred =  red[m:] -  red[:-m]
                dared= ared[m:] - ared[:-m]
                dir  =   ir[m:] -   ir[:-m]
                dair =  air[m:] -  air[:-m]

                y = numpy.abs(dir + dair + dred + dared)
                #y = (dir + dair + dred + dared)
                try:
                    oafl = self.artflev[-1]
                    if y[-1] > oafl:
                        nafl = (990.*oafl  + 25*y[-1])/1000.
                    else:
                        nafl = (90.*oafl  + 25.*y[-1])/100.
                    while len(self.artflev) < len(self.times[4]):
                        self.artflev.append(nafl)
                except AttributeError:
                    self.artflev = [0.]
                except IndexError:
                    self.artflev = list(2.5*y)
                y2 = self.artflev[-n+m:]
            
            #alldata  = [self.data[1],  self.data[2],  self.data[3],  self.data[4]]
            #alltimes = self.serialManager.times]*4[self.times[1], self.times[2], self.times[3], self.times[4]]
            sm = self.serialManager
            self.data['rawtimes'] = sm.times
            if not sm.times:
                #print "no data"
                return
            self.mainpanel.plot1.Update([], [], colors=['brown', 'purple', 'red', 'blue'], datadict=self.data)
            self.mainpanel.plot2.Update(numpy.array(self.data[1][-n:]) - numpy.array(self.data[2][-n:]), sm.times[-n:], datadict=self.data)
            self.mainpanel.plot3.Update(self.data[2][-n:], sm.times[-n:], datadict=self.data)
            #self.mainpanel.plot3.Update([self.data[2], self.data[4]], [self.times[2], self.times[4]], colors=['purple', 'blue'], n=n, datadict=self.data)
            self.mainpanel.plot4.Update(numpy.array(self.data[3][-n:]) - numpy.array(self.data[4][-n:]), sm.times[-n:], datadict=self.data)
            #self.mainpanel.plot5.Update([y, y2], [self.times[4][-n+m:], self.times[4][-n+m:]], colors=['blue', 'orange'], datadict=self.data)
            #self.mainpanel.plot5.Update((ir+3*air) - (numpy.mean(ir) + 3*numpy.mean(air)), self.times[4][-n:], datadict=self.data)
            self.mainpanel.plot5.Update(self.data[4][-n:], sm.times[-n:], datadict=self.data)
            self.mainpanel.plot6.Update(self.data[7][-n:], sm.times[-n:], datadict=self.data)
            
#            self.mainpanel.plot1.Update([], [], colors=['brown', 'purple', 'red', 'blue'], datadict=self.data)
#            self.mainpanel.plot2.Update(numpy.array(self.data[1][-n:]) - numpy.array(self.data[2][-n:]), self.times[1][-n:], datadict=self.data)
#            self.mainpanel.plot3.Update(self.data[2][-n:], self.times[2][-n:], datadict=self.data)
#            #self.mainpanel.plot3.Update([self.data[2], self.data[4]], [self.times[2], self.times[4]], colors=['purple', 'blue'], n=n, datadict=self.data)
#            self.mainpanel.plot4.Update(numpy.array(self.data[3][-n:]) - numpy.array(self.data[4][-n:]), self.times[3][-n:], datadict=self.data)
#            #self.mainpanel.plot5.Update([y, y2], [self.times[4][-n+m:], self.times[4][-n+m:]], colors=['blue', 'orange'], datadict=self.data)
#            #self.mainpanel.plot5.Update((ir+3*air) - (numpy.mean(ir) + 3*numpy.mean(air)), self.times[4][-n:], datadict=self.data)
#            self.mainpanel.plot5.Update(self.data[4][-n:], self.times[4][-n:], datadict=self.data)
#            self.mainpanel.plot6.Update(self.data[7][-n:], self.times[7][-n:], datadict=self.data)
            
        except ValueError: # this too shall
            traceback.print_exc()
        except:
            traceback.print_exc()

    def UpdateWaterfall(self):
        if not self.waterfall:
            return
        HEGlastsecond = numpy.mean(self.data[7][-10:])
        HEGlastminute = numpy.mean(self.data[7][-900:])
        HEGcurrent    = numpy.mean(self.data[7][-3:])
        dy  = int(min(200., max(-200., 600.*(HEGcurrent - HEGlastsecond) + .5)))
        hue = int(min(13., max(-10., 4.*(HEGcurrent - HEGlastsecond) + .5)))
        rgb = [11+hue, 1, 11-hue]
        self.waterfall.SetDY(dy)
        self.waterfall.SetRGB(rgb)
        
    def OnClose(self, evt=None):
        self.Disconnect()
        self.mainpanel.Destroy()
        sys.exit()
    
    def ToggleConnection(self, evt=None):
        if hasattr(self, 'serialManager') and self.serialManager.connected:
            self.Disconnect()
            self.timer.Stop()
            self.timer.Start(3000)
            self.mainpanel.connectbtn.SetLabel("Connect")
        else:
            self.Connect()
            self.timer.Stop()
            self.timer.Start(100)
            self.mainpanel.connectbtn.SetLabel("Disconnect")
    
    def Connect(self, evt=None, port=None):
        if USE_PARSERS:
            if not port:
                if '--port' in sys.argv:
                    try:
                        port = sys.argv[sys.argv.index('--port') + 1]
                        try:
                            port = int(port)
                        except:
                            pass
                    except:
                        pass
            if not port:
                ports = parsers.detect_protocols()
                if len(ports) != 1:
                    print "WARNING: detect_protocols found ", ports
                    print "Trying first port", ports[0][0]
                if ports: port = ports[0][0]

            if port:
                if DEBUG: print port
                self.serialManager = parsers.AutoHEG(port=port, callback=self.OnNewData, fast=fast)
                self.serialManager.start()
        else:
            self.serialManager = SerialManager(callback=self.OnNewData)
            self.serialManager.start()
        if '--wport' in sys.argv and len(sys.argv) > sys.argv.index('--wport')+1:
            self.waterfall = WaterfallController()
        else:
            self.waterfall = None
    def OpenSession(self, evt=None):
        dialog = wx.FileDialog(parent=self.mainpanel, message='', defaultDir=sessionsdir, 
                               wildcard='HEG files (*.pnb;*.pea;*.heg;*.heg.zip)|*.pnb;*.pea;*.heg;*.heg.zip')
        res = dialog.ShowModal()
        if res == wx.ID_OK:
            self.Disconnect()
            self.Clear()
            print dialog.GetPath()
            self.Connect(port=dialog.GetPath())
        


    def SaveSession(self, evt=None):
        if not os.path.exists(sessionsdir): os.mkdir(sessionsdir)
        usersessionsdir = os.path.join(sessionsdir, self.user)
        if not os.path.exists(usersessionsdir): os.mkdir(usersessionsdir)
        dlg = wx.FileDialog(parent=self.mainframe, 
                            message='Enter a filename to save HEG data into.', 
                            defaultFile="%s - %04i.%02i.%02i %02ih%02im - " % ((self.user, ) + time.localtime()[0:5]),
                            defaultDir=usersessionsdir, 
                            wildcard=heg_wildcard,
                            style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            sessionfile = dlg.GetPath()
            
            # win32's FileDialog appears to append the first wildcarded 
            # extension if no extension is given. We don't want that.
            if sys.platform == 'win32':
                if sessionfile.endswith('.pnb'):
                    sessionfile = sessionfile[:-4]
                    
            # File extensions will be appended in parsers.py, so let's not add
            # them here any more. 
            #if not sessionfile.lower()[-4:] in ('.pnb', '.pea', '.heg'):
            #    protocol = 'pea' #'heg'
            #    if self.heg:
            #        protocol = self.heg.file_extension
            #    #sessionfile = sessionfile + '.' + protocol
                
            dlg.Destroy()
            if DEBUG: print 'self.heg.LogRawToFile(%s)' % sessionfile
            if self.serialManager: # FIXME: what if not self.heg?
                self.serialManager.LogRawToFile(sessionfile) 
            return True

    def Disconnect(self, evt=None):
        try:
            self.serialManager.keeprunning = False
            self.serialManager.join(timeout=10)
            self.serialManager.Close()
        except:
            if DEBUG: traceback.print_exc()
        if self.waterfall: 
            self.waterfall.Close()
            self.waterfall = None
        return True
    
    def OpenCrust(self, evt=None):
        
        #cf = wx.py.crust.CrustFrame()
        cf = wx.Frame(parent=None)
        cr = wx.py.crust.Crust(cf)
        
        self.crustframes.append((cf, cr))
        cf.Show()
    
    def OpenFilling(self, evt=None):
        self.fillings.append(wx.py.filling.FillingFrame())
        self.fillings[-1].Show()
 
class WaterfallController:
    def __init__(self, port=None):
        self.starttime = time.time()
        self.initialized = False
        self.dy = 0
        self.rgb = [11, 11, 11]
        self.port = port
        self.ser = None
        self.InitSerialPort(port)
        self.decimation = 8
        self.i = 0
    
    def InitSerialPort(self, port=None):
        if not port:
            if self.port:
                port = self.port
            elif '--wport' in sys.argv:
                try:
                    port = int(sys.argv[sys.argv.index('--wport')+1])
                    self.port = port
                except:
                    traceback.print_exc()
        if not port:
            print "No serial port configured for the waterfall. Hope that's okay with you."
            return
        try:
            self.ser = serial.Serial(port=port-1, baudrate=57600, timeout=1)
            self.ser.flushInput()
            self.initialized = True
        except:
            print "Couldn't open the serial port for the waterfall. Extended error message:"
            traceback.print_exc()
        
    def SetDY(self, dy):
        dy = self.dy + max(-5, min(5, dy - self.dy))
        if (time.time() - self.starttime < 5.):
            return
        self.i += 1
        if (self.i % self.decimation):
            return
        if not (self.i % (self.decimation**self.decimation)):
            self.ser.write('g*')
            self.dy = 0
            #self.rgb[0] = 55
        if not self.initialized: return
        while self.dy < dy:
            self.ser.write('+')
            #print '+',
            self.dy += 1
        while self.dy > dy:
            self.ser.write('-')
            #print '-',
            self.dy -= 1
            
        print self.dy, self.rgb
    
    def SetRGB(self, rgb):
        if 1:#((self.i + self.decimation/2) % self.decimation):
            return
        if not self.initialized: return
        i = ((self.i/self.decimation) % 4)-1
        #for i in (0,1,2):
        if 1:
            rgb[i] = self.rgb[i] + max(-2, min(2, rgb[i] - self.rgb[i]))
            rgb[i] = min(60, max(1, rgb[i]))
            if self.rgb[i] != rgb[i]:
                self.ser.write('rgb'[i])
                while self.rgb[i] < rgb[i]:
                    #print '3',
                    self.ser.write('3')
                    self.rgb[i] += 1
                while self.rgb[i] > rgb[i]:
                    self.ser.write('5')
                    #print '5',
                    self.rgb[i] -= 1
    
    def Close(self):
        print "Shutting down the waterfall COM port..."
        if not self.initialized:
            print "Waterfall not initialized to start with. Nothing to do here."
            return
        self.ser.close()
        self.initialized = False


            


def main():
    global app
    app = MyApp()
    app.MainLoop()

#import cProfile
#cProfile.run('main()')
        
if __name__ == '__main__':
    if '--profile' in sys.argv:
        print 'profiling'

    else:
        main()
