__VERSION__ = version = '0.4'

import wx, numpy, thread, shelve, platform, traceback, subprocess
import os, sys, time, random, math

import parsers
import plotcanvas

import __main__
from __main__ import DEBUG, USE_HEGINPUT, opj

def mean(vals):
    if len(vals) > 0:
        return sum(vals) / float(len(vals))
    else:
        return vals

class PreviewPanel(wx.Panel):
    def __init__(self, path, *args, **kwargs):
        wx.Panel.__init__(self, *args, **kwargs)
        #self.filenametext    = wx.StaticText(parent=self, label=os.path.basename(path))
        self.baselinetext    = wx.StaticText(parent=self, label='Baseline: \t\t\t            ')
        self.averagetext     = wx.StaticText(parent=self, label='Average:  \t')
        self.endtext         = wx.StaticText(parent=self, label='Final:    \t')
        self.loadingtext     = wx.StaticText(parent=self, label='Loading:  \t %s' % os.path.basename(path))

        self.FOVtitletext    = wx.StaticText(parent=self, label='')
        self.FOVbaselinetext = wx.StaticText(parent=self, label='')
        self.FOVaveragetext  = wx.StaticText(parent=self, label='')
        self.FOVendtext      = wx.StaticText(parent=self, label='')

        if not hasattr(self, 'handbmp'):
            handimg  = plotcanvas.plot.Hand.GetImage()
            handimg.Rescale(16, 22, wx.IMAGE_QUALITY_HIGH)
            PreviewPanel.handbmp = handimg.ConvertToBitmap(32)
        if not hasattr(self, 'ptrbmp'):
            PreviewPanel.ptrbmp = wx.Image(opj('res', 'pointer.png')).ConvertToBitmap(32)
        if not hasattr(self, 'zoombmp'):
            PreviewPanel.zoombmp  = wx.Image(opj('res', 'zoom.png')).ConvertToBitmap(32)
        if not hasattr(self, 'trashbmp'):
            PreviewPanel.trashbmp = wx.Image(opj('res', 'grave.png')).ConvertToBitmap(32)# 'trashcan.png'
        
        self.ptrbtn   = wx.lib.buttons.GenBitmapToggleButton(parent=self, bitmap=self.ptrbmp)
        self.handbtn  = wx.lib.buttons.GenBitmapToggleButton(parent=self, bitmap=self.handbmp)
        self.zoombtn  = wx.lib.buttons.GenBitmapToggleButton(parent=self, bitmap=self.zoombmp)
        self.trashbtn = wx.lib.buttons.GenBitmapButton      (parent=self, bitmap=self.trashbmp)
        
        self.ptrbtn.Bind  (wx.EVT_BUTTON, lambda evt: (self.ClearBtns(), self.plot.SetEnableZoom(False), self.plot.SetEnableDrag(False)))
        self.handbtn.Bind (wx.EVT_BUTTON, lambda evt: (self.ClearBtns(), self.plot.SetEnableDrag(True)))
        self.zoombtn.Bind (wx.EVT_BUTTON, lambda evt: (self.ClearBtns(), self.plot.SetEnableZoom(True)))
        self.trashbtn.Bind(wx.EVT_BUTTON, self.SendToTrash)
        
        bgcolor = self.GetBackgroundColour()
        for btn in (self.ptrbtn, self.handbtn, self.zoombtn, self.trashbtn):
            btn.SetBackgroundColour(bgcolor)
            btn.SetForegroundColour(bgcolor)
            btn.SetUseFocusIndicator(False)
        
        self.sizer = wx.GridBagSizer(vgap=0, hgap=0)
        self.plot = plotcanvas.PlotCanvas(self, color='black')
        if map(int, wx.version().split(' ')[0].split('.')) < [2, 8, 10]:
            oldwxplot = True
        else:
            oldwxplot = False
        if not oldwxplot:
            self.plot.SetForegroundColour('black')
            self.plot.SetBackgroundColour(self.GetBackgroundColour())
            self.plot.SetGridColour('black')
            self.plot.SetYSpec('auto')
            self.plot.SetEnableAntiAliasing(True)
            self.plot.SetEnableHiRes(True)
            self.plot.SetXSpec('min')
            self.plot.SetFontSizeTitle(12)
            #self.plot.SetEnableZoom(True)
        else:
            self.plot.SetYSpec('auto')

        # Workaround for a bug which causes scrollbars to appear when they shouldn't
        self.plot.SetShowScrollbars = lambda x: None
        self.plot.SetMinSize((700, 250))
        #self.plot.SetBestFittingSize((2048, 500))
        
        self.sizer.Add(self.plot, pos=(0,4), span=(11,1), border=0, flag=wx.EXPAND)
        #self.sizer.Add(self.filenametext, pos=(0,2), span=(1,1), border=0)
        self.sizer.Add(self.baselinetext,    pos=(2,0),  span=(1,4), border=0)
        self.sizer.Add(self.averagetext,     pos=(3,0),  span=(1,4), border=0)
        self.sizer.Add(self.endtext,         pos=(4,0),  span=(1,4), border=0)
        self.sizer.Add(self.loadingtext,     pos=(1,0),  span=(1,4), border=0)
        self.sizer.Add(self.FOVtitletext,    pos=(7,0),  span=(1,4), border=0)
        self.sizer.Add(self.FOVbaselinetext, pos=(8,0),  span=(1,4), border=0)
        self.sizer.Add(self.FOVaveragetext,  pos=(9,0),  span=(1,4), border=0)
        self.sizer.Add(self.FOVendtext,      pos=(10,0), span=(1,4), border=0)
        self.sizer.Add(self.ptrbtn,          pos=(0,0),  span=(1,1), border=0)
        self.sizer.Add(self.handbtn,         pos=(0,1),  span=(1,1), border=0)
        self.sizer.Add(self.zoombtn,         pos=(0,2),  span=(1,1), border=0)
        self.sizer.Add(self.trashbtn,        pos=(0,3),  span=(1,1), border=0)
        
        
        
        self.SetSizerAndFit(self.sizer)
        self.sizer.Layout()
        #self.plot.Update([0., 0.], xLabel="Time (minutes)", yLabel="HEG ratio", title=os.path.basename(path))
        self.running = 0
        self.status = 0
        self.path = path
    
        # Intercept mouse events for zooming to trigger text updates
        # We have to catch the events where they're generated, or else they
        # get handled in plot.PlotCanvas.
        self.plot.canvas.Bind(wx.EVT_LEFT_DOWN, self.OnMouseLeftDown)
        self.plot.canvas.Bind(wx.EVT_LEFT_UP, self.OnMouseLeftUp)
        #self.plot.Bind(wx.EVT_MOTION, self.OnMotion)
        self.plot.canvas.Bind(wx.EVT_LEFT_DCLICK, self.OnMouseDoubleClick)
        self.plot.canvas.Bind(wx.EVT_RIGHT_DOWN, self.OnMouseRightDown)
        
        #self.Bind(wx.EVT_IDLE, self.UpdateText)

    def OnMouseLeftDown(self, evt=None):
        self.plot.OnMouseLeftDown(evt)
        self.UpdateText()
    def OnMouseLeftUp(self, evt=None):
        self.plot.OnMouseLeftUp(evt)
        self.UpdateText()
    def OnMouseDoubleClick(self, evt=None):
        self.plot.OnMouseDoubleClick(evt)
        self.UpdateText()
    def OnMouseRightDown(self, evt=None):
        self.plot.OnMouseRightDown(evt)
        self.UpdateText()
        
    def start(self):
        self.running = 1
        thread.start_new(self.Update, ())
        
    def Update(self):
        path = self.path
        #if DEBUG: print "starting processing of %s" % path

        if USE_HEGINPUT:
            heg = heginput.hegstream()
            heg.initialize(port=path, fast=1)
            heg.close()
            self.running = 0
        else:
            self.heg = heg = parsers.AutoHEG(port=path, fast=3)
            heg.start()
            heg.join()
            try:
                #heg.mgr.join()
                while not heg.mgr == None and heg.mgr.isAlive():
                    time.sleep(.05)
                heg.Close()
                self.running = 0
                #if DEBUG: print "finished with %s" % path, "hegdatalen = ", len(heg.hegdata)
            except:
                if DEBUG: traceback.print_exc()
                heg.Close()
                self.running = 0
                try: self.plot.Destroy()
                except: pass
                return
        self.status = 1
        #if DEBUG: print "starting processing of %s" % path, self.running, self.status

        
    def UpdatePlot(self):
        self.status = 2

        heg = self.heg
        path = self.path
        #if DEBUG: print "started displaying %s" % path
        if not heg.hegdata == None and len(heg.hegdata) > 2:
            self.loadingtext.SetLabel('')
            try:
                self.plot.Update(heg.hegdata, heg.times, xLabel="Time (minutes)", 
                                 yLabel="HEG ratio", title=os.path.basename(path),
                                 decimate=False)
            except OverflowError:
                print path
                traceback.print_exc()
                print heg.hegdata[:10], " ... ", heg.hegdata[-10:]
            except wx.PyDeadObjectError:
                return
            self.UpdateText()
            self.sizer.Layout()
            self.plot.Show(True)
            self.UpdateText()
            self.sizer.Layout()
        elif DEBUG: print `heg.hegdata`
        #if DEBUG: print "finished displaying %s" % path
    
    def UpdateText(self, evt=None):
        if not self.status == 2:
            return
        heg = self.heg
        baseline = max(heg.hegdata[0], 1.)
        end = heg.hegdata[-1]
        average = mean(heg.hegdata)
        self.baselinetext.SetLabel('Base:  \t%3.2f' % baseline)
        self.averagetext.SetLabel ('Mean:  \t%3.2f   (%2.1f%%)' % (average, 100.*(average/baseline-1)))
        self.endtext.SetLabel     ('Final: \t%3.2f   (%2.1f%%)' % (end,     100.*(end/baseline-1)))
        
        x0,x1 = self.plot.GetXCurrentRange()
        y0,y1 = self.plot.GetYCurrentRange()
        maxx0, maxx1 = self.plot.GetXMaxRange()
        maxy0, maxy1 = self.plot.GetYMaxRange()
        
        data_times = [((heg.times[i] - heg.times[0])/60., heg.hegdata[i]) for i in range(len(heg.hegdata))]
        
        leftovers  = filter(lambda dt: dt[0] >= x0 and dt[0] <= x1 and
                                       dt[1] >= y0 and dt[1] <= y1,
                            data_times)
        leftovers  = [l[1] for l in leftovers]
        try:
            baseline = max(leftovers[0], 1.)
            end = leftovers[-1]
            average = mean(leftovers)
            self.FOVtitletext.SetLabel   ('Field of view only:')
            self.FOVbaselinetext.SetLabel('Base:  \t%3.2f' % baseline)
            self.FOVaveragetext.SetLabel ('Mean:  \t%3.2f   (%2.1f%%)' % (average, 100.*(average/baseline-1)))
            self.FOVendtext.SetLabel     ('Final: \t%3.2f   (%2.1f%%)' % (end,     100.*(end/baseline-1)))
        #except IndexError:
        #    self.FOVbaselinetext.SetLabel('Base:  \tN/A')
        #    self.FOVaveragetext.SetLabel ('Mean:  \tN/A')
        #    self.FOVendtext.SetLabel     ('Final: \tN/A')
        except ValueError:
            pass
        self.sizer.Layout()
    def ClearBtns(self, evt=None):
        for btn in (self.ptrbtn, self.handbtn, self.zoombtn):
            btn.SetToggle(False)
    def SendToTrash(self, evt=None):
        dlg = wx.MessageDialog(parent=self, caption="Confirm Delete",
                        message="Move this session to the graveyard?", 
                        style=wx.OK|wx.CANCEL|wx.ICON_QUESTION)
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            graveyard = opj(__main__.sessionsdir, __main__.app.user, 'Trash')
            if not os.path.exists(graveyard):
                os.mkdir(graveyard)
                print "Made %s" % graveyard
                
            for fn in (self.path, self.path[:-4]+'.tmg'):
                dst = opj(graveyard, os.path.basename(fn))
                try:
                    os.rename(fn, dst)
                except:
                    if fn == self.path:
                        wx.MessageDialog(self, "Operation failed.").ShowModal()

            # parent should be wx.ScrolledWindow; grandparent should be ReviewFrame
            gramps = self.GetParent().GetParent()
            gramps.panels.remove(self)
            sizer = self.GetParent().GetSizer() 
            sizer.Remove(self)
            self.Destroy()
            gramps.Relayout()
            #sizer.Layout() 

class ReviewFrame(wx.Frame):
    def __init__(self, path, *args, **kwargs):
        if not 'size' in kwargs:
            kwargs['size'] = (950, 600)
        if not 'title' in kwargs:
            kwargs['title'] = "Session History - %s - WORKING; please be patient" % path
        if not 'style' in kwargs:
            kwargs['style'] = wx.DEFAULT_FRAME_STYLE #| wx.VSCROLL | wx.HSCROLL
        wx.Frame.__init__(self, *args, **kwargs)
        self.panel = wx.ScrolledWindow(parent=self, style=wx.TAB_TRAVERSAL|wx.NO_BORDER)
        self.panel.SetScrollRate(10,10)
        
        if DEBUG: print "Looking for files."
        files = [f for f in os.listdir(path) if f.lower()[-3:] in parsers.file_extensions
                                                and os.stat(opj(path, f)).st_size > 6144] # 6144 is 10 KiB or about 10 seconds.
        if DEBUG: print "File list built."
        #if DEBUG: print files
        #self.sizer = wx.GridBagSizer(vgap=5)
        self.Show(True)

        wait = wx.MessageDialog(self, message="This might take some time...")
        wait.Show()
        self.panels = []
        #for f,i in zip(files, range(len(files))):
        #    self.panels.append(PreviewPanel(opj(path, f), parent=self.panel))

        #wait.Destroy()

        #self.FitInside()
        #self.sizer.Layout()
        self.panel.SetScrollRate(10,10)
        #self.panel.SetBackgroundColour('black')
        self.Relayout()
        self.running = True
        self.Bind(wx.EVT_IDLE, self.DrawPanels)
        self.FillPanels(path, files)#thread.start_new(self.FillPanels, (path, files,))
        self.SetTitle("Session History - %s" % path)

    def Relayout(self):
        size = self.panel.GetSize()
        hsp  = self.panel.GetScrollPos(wx.HORIZONTAL)
        vsp  = self.panel.GetScrollPos(wx.VERTICAL)
        if hasattr(self, 'sizer'):
            self.SetSizer(None)
            self.sizer.Destroy()
        self.sizer = wx.BoxSizer(orient=wx.VERTICAL)
        self.panel.SetSizerAndFit(self.sizer)
        for p in self.panels:
            self.sizer.Add(p, flag=wx.EXPAND)#, pos=(i,0), border=1)    
        self.sizer.Layout()
        self.sizer.Fit(self.panel)
        self.Layout()
        self.SetSize(self.GetSize())
        self.panel.SetSize(size)
        self.panel.SetScrollPos(wx.HORIZONTAL, hsp)
        self.panel.SetScrollPos(wx.VERTICAL, vsp)

        

    def DrawPanels(self, evt=None):
        if [p.running for p in self.panels]:
                #time.sleep(.1)
                for p in self.panels:
                    if p.status == 1:
                        p.UpdatePlot()
                        break
        else:
            self.Bind(wx.EVT_IDLE, None)
            if DEBUG: print "All sessions displayed."


    def FillPanels(self, path, files):
        if DEBUG:
            start = time.time()
        try:
            for f,i in zip(files, range(len(files))):
                if not self.running:
                    return
                wx.Yield()
                self.panels.append(PreviewPanel(opj(path, f), parent=self.panel))
                self.sizer.Add(self.panels[-1], flag=wx.EXPAND)
                self.sizer.Layout()
                self.panel.AdjustScrollbars()
                self.panel.SetSize(self.panel.GetSize())
                self.panels[-1].start()
            #for pan in self.panels:
                while sum([p.running for p in self.panels]) > 2:
                    time.sleep(.1)
                    wx.Yield()
                    for p in self.panels:
                        if p.status == 1:
                            p.UpdatePlot()
                    #if [1 for p in self.panels if p.status != 2]:
                    #    break
                #for p in self.panels:
                #    if p.status == 1:
                #        p.UpdatePlot()
                #pan.start()
                #print 'starting pan'
                #self.Relayout()

                
            while sum([p.running for p in self.panels]) and self.running:
                time.sleep(.1)
                for p in self.panels:
                    if p.status == 1:
                        p.UpdatePlot()
            #if DEBUG: print "All sessions displayed."
            self.panel.AdjustScrollbars()
            self.Relayout()

        except wx.PyDeadObjectError:
            if DEBUG: print "FillPanels was still running at shutdown."
        if DEBUG:
            print "FillPanels took %3.3f seconds" % (time.time() - start)
        self.sizer.Layout()
    
    def Quit(self):
        self.running = False
        
    def Destroy(self, *args, **kwargs):
        self.running = False
        wx.Frame.Destroy(self, *args, **kwargs)
