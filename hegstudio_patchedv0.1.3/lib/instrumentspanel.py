from __main__ import USE_MPLAYER, DEBUG, mediadir, opj, USE_THREADS, error

import wx, time, os, thread, sys
import plotcanvas

def mean(vals):
    if len(vals) > 0:
        return sum(vals) / float(len(vals))
    else:
        return vals

class InstrumentsPanel(wx.Panel):
    def __init__(self, parent, id, app, blind=False, *args, **kwargs):
        self.app = app
        
        fgcolor = 'white'
        bgcolor = 'black'
        self.parent = parent
        self.starttime = time.time()
        self.baseline = None
        if not 'style' in kwargs:
            kwargs['style'] = 0
        kwargs['style'] |= wx.FULL_REPAINT_ON_RESIZE
        self.updatetimer, self.hegratio, self.percentchange, self.sessionchange = (0.,)*4
        wx.Panel.__init__(self, parent, id, *args, **kwargs)
        self.SetDoubleBuffered(True)
        self.SetBackgroundColour('black')
        self.SetForegroundColour('white')
                
        sizer = self.sizer = wx.GridBagSizer(1,1)
        self.SetSizer(sizer)
        
        r,c = 0,0
        self.plot = plotcanvas.PlotCanvas(self)#, id=-1)
        oldwxplot = False
        try:
            if map(int, wx.version().split(' ')[0].split('.')) < [2, 8, 10]:
                oldwxplot = True
        except: # beta version of wxpython
            pass
        if not oldwxplot:
            self.plot.SetForegroundColour('gray')
            self.plot.SetGridColour('blue')
            self.plot.SetBackgroundColour('black')
            self.plot.SetYSpec('auto4')
            self.plot.SetEnableAntiAliasing(True)
            self.plot.SetEnableHiRes(True)
            self.plot.SetXSpec('min')
        else:
            self.plot.SetYSpec('auto')

        # Workaround for a bug which causes scrollbars to appear when they shouldn't
        self.plot.SetShowScrollbars = lambda x: None
                
        self.SetAveragingInterval()
        
        self.plot.Update([100., 100.], [0., 1.])
        
        sizer.Add(self.plot, (r,c), (4,20))
        
        r += 4
        
        self.mediarc = r,c

        c = 16
        
        btnnames = ['play', 'record', 'pause', 'stop']
        self.sessbtns = {}
        for name in btnnames:
            onimg  = wx.Image(opj('res', name+'-on.png'))
            offimg = wx.Image(opj('res', name+'-off.png'))
            onimg.Rescale (24, 24, wx.IMAGE_QUALITY_HIGH)
            offimg.Rescale(24, 24, wx.IMAGE_QUALITY_HIGH)
            onbmp  = onimg.ConvertToBitmap(32)
            offbmp = onimg.ConvertToBitmap(32)
            btn = wx.lib.buttons.GenBitmapToggleButton(parent=self, id=-1, bitmap=offbmp)
            btn.SetBitmapSelected(onbmp)
            btn.SetBackgroundColour(bgcolor)
            btn.SetForegroundColour(bgcolor)
            btn.SetUseFocusIndicator(False)
            sizer.Add(btn, (r,c), (1,1))
            self.sessbtns[name] = btn      
            c += 1
       
        self.sessbtns['play'].SetToggle(True)
        self.sessbtns['play'].Enable(False)
        #if not '--noautosave' in sys.argv: self.sessbtns['record'].SetToggle(True)
        
        self.sessbtns['play'  ].Bind(wx.EVT_BUTTON, self.OnPlay)
        self.sessbtns['record'].Bind(wx.EVT_BUTTON, self.OnRecord)
        self.sessbtns['pause' ].Bind(wx.EVT_BUTTON, self.OnPause)
        self.sessbtns['stop'  ].Bind(wx.EVT_BUTTON, self.OnStop)

        c -= 4    
        r += 1
       
        self.timertitletext = wx.StaticText(parent=self, id=-1, label="Session time: ")
        sizer.Add(self.timertitletext, (r,c), (1,4))
        r += 1
        self.datatimertext = wx.StaticText(parent=self, id=-1, label='00:00')
        sizer.Add(self.datatimertext, (r,c), (1,4))
        r += 1

        self.hegratiotitletext = wx.StaticText(parent=self, id=-1, label="HEG ratio: ")
        sizer.Add(self.hegratiotitletext, (r,c), (1,4))
        r += 1
        self.hegratiotext = wx.StaticText(parent=self, id=-1, label=" 0.0")
        sizer.Add(self.hegratiotext, (r,c), (1,4))
        r += 1

        self.hegbaselinetitletext = wx.StaticText(parent=self, id=-1, label="HEG baseline: ")
        sizer.Add(self.hegbaselinetitletext, (r,c), (1,4))
        r += 1
        self.hegbaselinetext = wx.StaticText(parent=self, id=-1, label=" 0.0")
        sizer.Add(self.hegbaselinetext, (r,c), (1,4))
        r += 1
        
        self.percentchangetitletext = wx.StaticText(parent=self, id=-1, label="Current gain: ")
        sizer.Add(self.percentchangetitletext, (r,c), (1,4))
        r += 1
        self.percentchangetext = wx.StaticText(parent=self, id=-1, label=" 0.0%")
        sizer.Add(self.percentchangetext, (r,c), (1,4))
        r += 1

        self.sessionchangetitletext = wx.StaticText(parent=self, id=-1, label="Session gain: ")
        sizer.Add(self.sessionchangetitletext, (r,c), (1,4))
        r += 1
        self.sessionchangetext = wx.StaticText(parent=self, id=-1, label=" 0.0%")
        self.sessionchangetext.SetForegroundColour('white')
        sizer.Add(self.sessionchangetext, (r,c), (1,4))
        r += 1
        
        
        #self.timertext.SetFont(wx.Font(24, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD))
        self.datatimertext.SetFont(wx.Font(24, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD))
        self.hegratiotext.SetFont(wx.Font(24, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD))
        self.hegbaselinetext.SetFont(wx.Font(24, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD))
        self.percentchangetext.SetFont(wx.Font(32, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD))
        self.sessionchangetext.SetFont(wx.Font(32, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD))
       
        self.datatimertext.SetDoubleBuffered(True)
        self.hegratiotext.SetDoubleBuffered(True)
        self.hegbaselinetext.SetDoubleBuffered(True)
        self.percentchangetext.SetDoubleBuffered(True)
        self.sessionchangetext.SetDoubleBuffered(True)
        
        self.timertitletext.SetForegroundColour(fgcolor)
        #self.timertext.SetForegroundColour('grey')
        #self.datatimertitletext.SetForegroundColour(fgcolor)
        self.datatimertext.SetForegroundColour(fgcolor)
        self.hegratiotitletext.SetForegroundColour(fgcolor)
        self.hegratiotext.SetForegroundColour('green')
        self.hegbaselinetitletext.SetForegroundColour(fgcolor)
        self.hegbaselinetext.SetForegroundColour(fgcolor)
        self.percentchangetitletext.SetForegroundColour(fgcolor)
        self.percentchangetext.SetForegroundColour('green')
        self.sessionchangetitletext.SetForegroundColour(fgcolor)
        self.sessionchangetext.SetForegroundColour('yellow')


        self.r, self.c = r, c
        
        sizer.Layout()
                
        
    def OnPlay(self, evt=None):
        if self.sessbtns['stop'].GetValue():
            self.Clear(reinit=False)
        self.sessbtns['stop'].SetToggle(False)
        self.sessbtns['stop'].Enable(True)
        self.sessbtns['play'].Enable(False)
        self.app.OnPlay()
    def OnRecord(self, evt=None):
        if not self.app.OnRecord(val=self.sessbtns['pause'].GetToggle()):
            self.sessbtns['record'].SetToggle(False)
    def OnPause(self, evt=None):
        self.app.OnPause(val=self.sessbtns['pause'].GetToggle())
    def OnStop(self, evt=None):
        self.sessbtns['play'].SetToggle(False)
        self.sessbtns['play'].Enable(True)
        self.sessbtns['stop'].Enable(False)
        self.sessbtns['record'].SetToggle(False)
        self.Clear(reinit=False)
        self.app.OnStop()           
        
    def Clear(self, evt=None, reinit=True):
        self.baseline = None
        self.starttime = time.time()
        if reinit and self.app.heg and self.app.heg.hegdata:
            app.ReInitHEG()
    
    def SetAveragingInterval(self, interval=1.0):
        "Set the duration for the moving average, in seconds."
        self.avelen = int(round(interval * 10.))
                
    def Update(self, data, times):
        self.UpdateData(data, times)
        self.UpdateText()
        self.UpdatePlot(data, times)

    def UpdateData(self, data, times):
        "data is expected to be ALL the data collected so far."
        if not data:
            return
        if not self.baseline:
            self.baseline = max(10., mean(data))
        self.hegratio = mean(data[-self.avelen:])
        self.percentchange = (self.hegratio - self.baseline) / self.baseline
        self.sessionchange = (mean(data) - self.baseline) / self.baseline
        self.t1 = (time.time() - self.starttime) # time elapsed
        #self.t2 = len(data) * .008192 * 4.       # seconds' worth of data we have
        self.t2 = self.t1

    def UpdateText(self, data=None, times=None):
        minutes1, seconds1 = int(self.t1/60), int(self.t1%60)
        minutes2, seconds2 = int((self.t2-self.app.paused_time)/60), int((self.t2-self.app.paused_time)%60)
        
        try:
            #self.timertext.SetLabel("%02i:%02i" % (minutes1, seconds1))
            self.datatimertext.SetLabel("%02i:%02i" % (minutes2, seconds2))
            self.hegratiotext.SetLabel("%5.1f" % self.hegratio)
            self.hegbaselinetext.SetLabel("%5.1f" % self.baseline)
            self.percentchangetext.SetLabel("%4.1f%%" % (self.percentchange*100))
            self.sessionchangetext.SetLabel("%4.1f%%" % (self.sessionchange*100))
        except wx.PyAssertionError:
            print "Assertion error."
            if DEBUG: traceback.print_exc()

    def UpdatePlot(self, data, times):
        if USE_THREADS:
            if not self.plot.mutex.locked():
                self.plot.mutex.acquire()
                thread.start_new(self.plot.Update, (data, times))#self.plot.Update(data)
        else:
            self.plot.Update(data, times)

    def UpdateMovie(self, data=[], times=[]):
        #This shouldn't be here. Oh well.
        pass
    
    def Destroy(self):
        self.OnClose()
        wx.Panel.Destroy(self)
        
    def OnResize(self, evt=None):
        totalframesize = self.parent.GetBestSize()
        self.plot.SetInitialSize((totalframesize[0], totalframesize[1]/5))
        self.sizer.Layout()
    
    def OnClose(self, evt=None):
        pass
