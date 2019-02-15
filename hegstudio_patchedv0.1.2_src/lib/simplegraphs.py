import wx, time, os, thread, sys, math, random, numpy, traceback
from __main__ import DEBUG, opj, USE_THREADS, AvailableModes
import plotcanvas
from instrumentspanel import InstrumentsPanel, mean


class SimpleGraphsPanel(InstrumentsPanel):
    def __init__(self, *args, **kwargs):
        InstrumentsPanel.__init__(self, *args, **kwargs)
        
        self.trendmeter = plotcanvas.BarGraphMeter(parent=self, width=40)
        self.trendmeter.SetYRange(ymin=-1., ymax=1.)
        self.trendmeter.SetForegroundColour('white')
        self.trendmeter.SetBackgroundColour('black')
        self.trendmeter.SetDoubleBuffered(True)
        self.trendmeter.SetXSpec('none')
        self.trendmeter.SetYSpec('none')            
        #self.trendmeter.title = 'HEG Trend'
        trendtext = wx.StaticText(parent=self, label="HEG trend:")
        self.sizer.Add(trendtext, (self.r, self.c), (1,4))
        self.r += 1
        self.sizer.Add(self.trendmeter, (self.r, self.c), (1,4), flag=wx.EXPAND)
        self.r += 1
        
        self.midplot = plotcanvas.PlotCanvas(self)
        if map(int, wx.version().split(' ')[0].split('.')) < [2, 8, 10]:
            oldwxplot = True
        else:
            oldwxplot = False
        if not oldwxplot:
            self.midplot.SetForegroundColour('blue')
            self.midplot.SetGridColour('blue')
            self.midplot.SetBackgroundColour('black')
            self.midplot.SetYSpec('min')
            self.midplot.SetEnableAntiAliasing(True)
            self.midplot.SetEnableHiRes(True)
            self.midplot.SetXSpec('min')
        else:
            self.midplot.SetYSpec('min')

        # Workaround for a bug which causes scrollbars to appear when they shouldn't
        self.midplot.SetShowScrollbars = lambda x: None
                
        self.Bind(wx.EVT_SIZE, self.OnResize)

        self.midplot.Update([100., 100.], [0., 1.])
        
        self.sizer.Add(self.midplot, self.mediarc, (16,16), flag=wx.EXPAND)
        self.OnResize()

    def OnResize(self, *args, **kwargs):
        totalframesize = self.parent.GetBestSize()
        self.midplot.SetInitialSize((totalframesize[0]-200, 2*totalframesize[1]/5))
        
        self.plot.SetInitialSize((totalframesize[0], 3*totalframesize[1]/10))
        self.sizer.Layout()
        #InstrumentsPanel.OnResize(self, *args, **kwargs)

    def UpdateMovie(self, data, times):
        self.midplot.Update(data[-320:], numpy.array(times[-320:]) - times[0], zero_x=False)
        
        delta = mean(data[-5:]) - mean(data[-50:])
        sigma = numpy.std(data[-300:])
        
        contrast = 1./(1+math.e**(-delta/sigma*5.))*100. - 85.
        zoom     = 1./(1+math.e**(-delta/sigma*2.))
        
        trend = 1./(1+math.e**(-delta/sigma)) * 2 - 1

        r = int((1-max(-trend, 0))*64)
        g = int((1-math.fabs(trend))*64)
        b = int((1-max( trend, 0))*64)
        if trend < 0:
            b = 95 + int(-trend*160)
        else:
            r = 95 + int(trend*160)
        self.trendmeter.SetColor(wx.Color(r,g,b))
        self.trendmeter.SetValue(trend)
        
        InstrumentsPanel.UpdateMovie(self, data, times)


AvailableModes['SimpleGraphs (beta)'] = SimpleGraphsPanel