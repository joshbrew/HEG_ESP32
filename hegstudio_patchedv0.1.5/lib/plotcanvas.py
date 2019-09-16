import sys, wx, numpy, wx.lib.plot, plot, thread, time
wx.lib.plot = plot

from __main__ import DEBUG, USE_THREADS

DEBUG = '--debug' in sys.argv

class PlotCanvas(plot.PlotCanvas):
    def __init__(self, parent, color='green'):
        plot.PlotCanvas.__init__(self, parent)
        self.mutex = thread.allocate_lock()
        self.color = color
        self.SetMinSize((200, 50))
        self.SetInitialSize((1280,420))
    
    def Downsample(self, vector, factor):
        """
        downsample(vector, factor):
            Downsample (by averaging) a vector by an integer factor.
        """
        rem = (len(vector) % factor)
        if rem:
            vector = vector[:-rem]
        vector.shape = (len(vector)/factor, factor)
        return numpy.mean(vector, axis=1)
    
    def Update(self, data, times, decimate=True, xLabel="Time (minutes)", yLabel="HEG ratio", title='', zero_x=True):
        t1 = time.time()
        #minpersample = .008192 / 60. * 4. # minutes per sample; ~= 30.5 hz
        #minpersample = 1. / 10.1 / 60.
        data = data[:]
        print data
        times = times[:]
        if len(data) < len(times): times = times[:len(data)]
        elif len(times) < len(data): data = data[:len(times)]
        data1 = numpy.zeros((len(data), 2))
        data1[:, 1]  = data
        data1[:, 0] = times
        if zero_x:
            data1[:, 0] = (data1[:, 0] - data1[0, 0])
        if not 'seconds' in xLabel:
            data1[:, 0] /= 60.
        #data1[:,0] = numpy.arange(len(data)) * minpersample
        
        #decimate = False
        if decimate:
            decimate = 1 + len(data)/2048
            data2 = numpy.zeros((len(data)/decimate, 2))
            data2[:,0] = self.Downsample(data1[:,0], decimate)
            data2[:,1] = self.Downsample(data1[:,1], decimate)
        else:
            data2 = data1
        #data2 = data1[::decimate, :]
        t2 = time.time()
        #lines = wx.lib.plot.PolyMarker(data2, colour=self.color, fillstyle=wx.TRANSPARENT)
        #lines = wx.lib.plot.PolySpline(data2, colour=self.color)
        lines = wx.lib.plot.PolyLine(data2, colour=self.color)

        graph = wx.lib.plot.PlotGraphics([lines], xLabel=xLabel, yLabel=yLabel, title=title)
        t3 = time.time()
        if wx.version() < '4':
            try:
                self.Draw(graph)
            except wx.PyDeadObjectError:
                pass
        else:
            self.Draw(graph)

        t4 = time.time()
        #if DEBUG: print "%1.4f %1.4f %1.4f" % (t4-t3, t3-t2, t2-t1), decimate, data1.shape, data2.shape
        if USE_THREADS:
            try:
                self.mutex.release()
            except thread.error: # usually happens due to the initial 
                                 # plot.Update([100., 100.]) call during init
                pass#if DEBUG:
                 #   print "Plot mutex not locked when release attempted."    
    #def GetXCurrentRange(self):
    #    xr = super(plot.PlotCanvas, self).GetXCurrentRange()
        
        
        
class BarGraphMeter(plot.PlotCanvas):
    def __init__(self, parent, color='red', width=40, *args, **kwargs):
        plot.PlotCanvas.__init__(self, parent, *args, **kwargs)
        self.color = color
        self.ymin, self.ymax = None, None
        self.width = width
        self.SetMinSize((40, 100))
        self.SetInitialSize((80,150))
        self.title = ''
        self.xlabel = ''
        self.ylabel = ''
    def SetYRange(self, ymin=None, ymax=None):
        self.ymin, self.ymax = ymin, ymax
    def SetColor(self, color):
        self.color = color
#    def _getCurrentRange(self):
#        """Returns (minY, maxY) y-axis range for displayed graph"""
#        if self.ymin != None and self.ymax != None:
#            if DEBUG: print self._axisInterval(self._ySpec, self.ymin, self.ymax)
#            return self._axisInterval(self._ySpec, self.ymin, self.ymax)
#        else:
#            return plot.PlotCanvas._getCurrentRange(self)
#    def _getYMaxRange(self):
#        """Returns (minY, maxY) y-axis range for displayed graph"""
#        if self.ymin != None and self.ymax != None:
#            if DEBUG: print self._axisInterval(self._ySpec, self.ymin, self.ymax)
#            return self._axisInterval(self._ySpec, self.ymin, self.ymax)
#        graphics= self.last_draw[0]
#        p1, p2 = graphics.boundingBox()     # min, max points of graphics
#        yAxis = self._axisInterval(self._ySpec, p1[1], p2[1])
#        return yAxis
    def SetValue(self, value):
        points1=[(1,0), (1,value)]
        line1 = wx.lib.plot.PolyLine(points1, colour=self.color, width=self.width)
        graph = wx.lib.plot.PlotGraphics([line1], self.title, self.xlabel, self.ylabel)
        if self.ymin != None and self.ymax != None:
            yAxis = (self.ymin, self.ymax)
        else:
            yAxis = None
        self.Draw(graph, yAxis=yAxis)