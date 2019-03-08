from __main__ import USE_MPLAYER, DEBUG, mediadir, opj, USE_THREADS, error, AvailableModes

import wx, time, os, thread, sys, traceback
import plotcanvas

from instrumentspanel import InstrumentsPanel, mean


if USE_MPLAYER:
    import MplayerCtrl
else:
    import wx.media


if USE_MPLAYER:
  class MPlayerMediaControl(MplayerCtrl.MplayerCtrl):
    def __init__(self, *args, **kwargs):
        self.cached_length = None
        if not 'mplayer_path' in kwargs:
            if sys.platform == 'win32':
                kwargs['mplayer_path'] = opj('res', 'mplayer.exe')
            elif sys.platform == 'darwin':
                kwargs['mplayer_path'] = opj('res', 'mplayer-darwin')
            else:
                kwargs['mplayer_path'] = opj('res', 'mplayer')
        kwargs['mplayer_args'] = ['-quiet']
        if not 'id' in kwargs:
            kwargs['id'] = -1
        if not 'sound' in kwargs:
            kwargs['sound'] = False
        #if not 'size' in kwargs:
        #    kwargs['size'] = (320, 240)
        kwargs['pausing'] = True
        self.res = None
        self.last_seek = 25.
        if DEBUG:
            self.seek_count = 0
            self.null_seek_count = 0
            self.first_seek = None
        
        super(MPlayerMediaControl, self).__init__(*args, **kwargs)
        
    def Seek(self, pos, fmt=2, filter=True):
        if fmt==1:
            super(MPlayerMediaControl, self).Seek(pos, 1)
            return
        if DEBUG:
            try:
                self.last_seek_time = time.time() 
            except:
                self.last_seek_time = time.time() 
        try:
            newpos = 100.*(pos / self.Length()) % 100.
            if self.last_seek and filter:
                if (self.last_seek - newpos)**2 < 100:
                    newpos = (self.last_seek + newpos)/2
                
            if newpos == self.last_seek:
                if DEBUG: self.null_seek_count += 1
                return
            else:
                if DEBUG:
                    self.seek_count += 1
                    if not self.first_seek: self.first_seek = time.time()
                    if not (self.seek_count % 100) and self.seek_count > 0:
                        print "Made %i seeks, skipped %i seeks in %4.3f seconds (%2.2f/sec)" % \
                            (self.seek_count, self.null_seek_count, time.time()-self.first_seek, 
                             self.seek_count/(time.time()-self.first_seek))
                self.last_seek = newpos
            super(MPlayerMediaControl, self).Seek(newpos, 1)
        except ZeroDivisionError:
            return

    def Play(self):
        pass#self.Pause()
    def Length(self):
        if not self.process_alive:
            if DEBUG: print "process not alive!"
            return 0
        if self.cached_length:
            return self.cached_length
        l = self.GetTimeLength()
        if l and type(l) in (float, int):
            self.cached_length = int(1000.*l)
            return self.cached_length
        else:
            return 0
    
    def GetBestSize(self):
        if self.res:
            return self.res
        try:
            res = self.GetVideoResolution()
        except:
            if DEBUG: print "Couldn't run self.GetVideoResolution()."
            res = None
        if res and not type(res) == float: # we often get video positions (floats) instead
            if type(res) == str:           # due to a MplayerCtrl bug
                try:
                    res = tuple(map(int, res.split(' x ')))
                except:
                    if DEBUG: 
                        traceback.print_exc()
            self.res = res
            return res
        else:
            #print "GetBestSize failed"
            #time.sleep(.1) # GetBestSize fails for a few seconds after a video loads
            return None
        
    def Load(self, path, driver=None):
        self.cached_length = None
        self.res = None
        if not self.process_alive:
            self.Start(media_file=path)
        else:
            self.cached_length = None
            self.Loadfile(path)
        self.Osd(0)
        #self.Pause()
        self.FrameDrop(1)
        self.Seek(25., 1, filter=False)
        self.GetBestSize()
        return True





class HEGFlightInstrumentsPanel(InstrumentsPanel):
    def __init__(self, parent, id, app, blind=False, reverse=False, *args, **kwargs):
        InstrumentsPanel.__init__(self, parent, id, app, blind=blind, *args, **kwargs)
        self.blind = blind
        self.reverse = reverse
        
        sizer = self.sizer
        fgcolor = self.GetForegroundColour()
        bgcolor = self.GetBackgroundColour()
        
        self.loaded = False
        if USE_MPLAYER:
            self.SetDoubleBuffered(False)
        
        if not os.path.exists(mediadir): os.mkdir(mediadir)
        extensions = ['.avi']
        if not USE_MPLAYER:
            extensions.extend(['.wmv', '.mp4', '.mpg', '.m4v'])
        self.mediafiles = [opj(mediadir, fn) for fn in os.listdir(mediadir) 
                      if fn.lower()[-4:] in extensions]
        
        self.mcmutex = thread.allocate_lock()

        self.SetAveragingInterval()
        self.SetSensitivity()


        r, c = self.r, self.c
        
        senstitle = wx.StaticText(parent=self, id=-1, label="Sensitivity: ")
        sizer.Add(senstitle, (r,c), (1,4))
        r += 1
        self.sens = sens = wx.SpinCtrl(parent=self, value=str(int(self.unadjusted_sensitivity)), min=1, max=1000)
        sens.Bind(wx.EVT_SPINCTRL, self.OnSensitivityCtrl)
        sizer.Add(sens, (r,c), (1,4))
        r += 1

        videostitle = wx.StaticText(parent=self, id=-1, label="Video: ")
        sizer.Add(videostitle, (r,c), (1,4))
        r += 1
        self.vids = vids = wx.Choice(parent=self, name="Video: ", choices=[os.path.basename(f) for f in self.mediafiles])
        vids.Bind(wx.EVT_CHOICE, self.OnVideoCtrl)
        sizer.Add(vids, (r,c), (1,4))
        r += 1
        
        feedbacktitle = wx.StaticText(parent=self, id=-1, label="Feedback mode: ")
        sizer.Add(feedbacktitle, (r,c), (1,4))
        r += 1
        
        feedbackchoices = ['Responsive', 'Smooth']
        self.feedback = feedback = wx.Choice(parent=self, name="Feedback mode: ", choices=feedbackchoices)
        feedback.SetSelection(feedbackchoices.index(self.app.userprofile['feedbackmode']))
        feedback.Bind(wx.EVT_CHOICE, self.OnFeedbackMode)
        sizer.Add(feedback, (r,c), (1,4))
        r += 1

        r += 1
                
        self.vids.SetFont(wx.Font(10, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL))
        videostitle.SetForegroundColour(fgcolor)
        feedbacktitle.SetForegroundColour(fgcolor)
        senstitle.SetForegroundColour(fgcolor)
        self.sens.SetForegroundColour(fgcolor)
        self.sens.SetBackgroundColour(bgcolor)
        self.vids.SetForegroundColour(fgcolor)
        self.vids.SetBackgroundColour(bgcolor)
        
        self.r, self.c = r,c
        
        self.InitMediaCtrl(attach=self.app.userprofile['mcattached'])
        
        sizer.Layout()
        
    def InitMediaCtrl(self, attach=True):
        self.CloseMC()
        
        if attach == 'toggle':
            attach = not self.app.userprofile['mcattached']
            self.mcsizer = self.sizer
        
        if attach:
            parent = self
        else:
            self.mcframe = wx.Frame(parent=None, id=-1, size=self.app.userprofile['mcframesize'])
            if self.app.userprofile['mcframemaximized']: self.mcframe.Maximize(True)
            self.mcframe.SetBackgroundColour(self.GetBackgroundColour())
            self.mcsizer = wx.BoxSizer()
            #self.mcframe.SetSizer(self.mcsizer)
            self.mcframe.Show(True)
            parent = self.mcframe
        
        if USE_MPLAYER:
            self.mc = MPlayerMediaControl(parent=parent, id=-1)
            #self.mc.Bind(lib.MplayerCtrl.EVT_MEDIA_STARTED, self.OnMplayerLoaded)
        else:
            if sys.platform == 'win32':
                try:
                    self.mc = wx.media.MediaCtrl(self, szBackend=wx.media.MEDIABACKEND_WMP10)
                except:
                    error("Couldn't load the Windows Media Player backend.  If you notice bugs, please install Windows Media Player. Avoid version 12; versions 10 and 11 work well.")
                    self.mc = wx.media.MediaCtrl(parent=parent)
            else:
                self.mc = wx.media.MediaCtrl(parent=parent)
            self.mc.Bind(wx.media.EVT_MEDIA_LOADED, self.OnMediaLoaded)
                    
        if self.LoadVideo(self.app.userprofile['video']): # True if error
            try:
                self.LoadVideo(random.choice(self.mediafiles))
            except:
                if len(self.mediafiles) < 1:
                    error("Couldn't find any video files in %s." % mediadir)

        
        if attach:
            self.sizer.Add(self.mc, self.mediarc, (30,16), flag=wx.ALIGN_CENTER_HORIZONTAL)
            self.Bind(wx.EVT_SIZE, self.OnResize)
        else:
            #self.mcsizer.Add(self.mc)
            self.mcframe.Bind(wx.EVT_SIZE, self.OnResize)
            self.mcframe.Bind(wx.EVT_CLOSE, lambda x: self.InitMediaCtrl(attach=True))
        self.app.userprofile['mcattached'] = attach
        self.mc.Bind(wx.EVT_CONTEXT_MENU, self.OnMCRightClick)

        self.sizer.Layout()


    def OnMCRightClick(self, evt=None):
        command = 'Attach'
        if self.app.userprofile['mcattached']: 
            command = 'Detach'
        
        menulist = [('Open &video', self.OpenVideo),
                    (command, lambda x: self.InitMediaCtrl(attach='toggle'))]
                    
        newmenu = wx.Menu()
        for item in menulist:
            newmenuitem = wx.MenuItem(parentMenu=newmenu, id=-1, text=item[0])
            self.Bind(wx.EVT_MENU, item[1], newmenuitem)
            newmenu.AppendItem(newmenuitem)
        self.PopupMenu(newmenu, self.ScreenToClient(evt.GetPosition()))

        
        
    def OpenVideo(self, evt=None):
        dialog = wx.FileDialog(parent=None, message='', defaultDir=mediadir, 
                               wildcard='Video files (*.avi;*.mpg;*.wmv;*.mp4;*.m4v)|*.avi;*.mpg;*.wmv;*.mp4;*.m4v')
        res = dialog.ShowModal()
        if res == wx.ID_OK:
            self.loaded = False
            self.LoadVideo(dialog.GetPath())

    def LoadVideo(self, path):
        if not os.path.exists(path) and os.path.exists(opj(mediadir, path)):
            path = opj(mediadir, path)
        elif not os.path.exists(path):
            return True
        self.loaded = False
        if DEBUG: print "Trying to load %s" % path
        if not self.mc.Load(path):
            wx.MessageBox("Unable to load %s: Unsupported format?" % path, "ERROR", wx.ICON_ERROR | wx.OK)
            return True
        self.mc.SetInitialSize()
        movies = [opj(mediadir, fn) if not os.path.dirname(fn) else fn for fn in self.vids.GetItems()]
        if not (path in movies or os.path.basename(path) in movies):
            self.vids.Append(path)
            movies.append(path)
        i = movies.index(path)
        self.vids.SetSelection(i)
        self.app.userprofile['video'] = path
        
        self.movieoffset = .25 # FIXME: load from an .ini file
        self.smoothmovieoffset = .25
        wx.CallLater(500, self.OnResize, self)
        wx.CallLater(1500, self.OnResize, self)
        wx.CallLater(2500, self.OnResize, self)
        return False
    
    def OnVideoCtrl(self, evt=None):
        path = opj(mediadir, evt.GetString())
        self.LoadVideo(path)

    def OnMplayerLoaded(self, evt=None):
        #print "OnMplayerLoaded called"
        pass
    def OnMediaLoaded(self, evt=None):
        #print "OnMediaLoaded called"
        if not USE_MPLAYER:
            self.mc.Play()
            self.mc.Pause()
        self.sizer.Layout()
        size = self.mc.GetBestSize()
        if size and size[0]:
            self.OnResize(evt)
            self.loaded = True
        self.SetSensitivity(self.unadjusted_sensitivity)

    def OnFeedbackMode(self, evt=None):
        oldpos = self.CalcMoviePos()
        self.app.userprofile['feedbackmode'] = evt.GetString()
        newpos = self.CalcMoviePos()
        if self.app.userprofile['feedbackmode'] == 'Smooth':
            self.smoothmovieoffset += oldpos  - newpos
        else: #if self.app.userprofile['feedbackmode'] == 'Responsive':
            self.movieoffset += oldpos - newpos
        

    def OnResize(self, evt=None):
        InstrumentsPanel.OnResize(self, evt)
        self.mc.SetInitialSize()
        #print self.mc.GetBestSize(), self.mc.Length()
        totalframesize = self.parent.GetBestSize()
        #self.plot.SetMinSize((totalframesize[0], totalframesize[1]/5))    
        framesize = totalframesize[0] - 200, totalframesize[1]*4/5
        if USE_MPLAYER:
            for i in range(10):
                try:
                    videosize = self.mc.GetBestSize()
                except Exception:
                    return
                if videosize:
                    break
                else:
                    videosize = (720, 400)
            controlsize = self.mc.GetSize()
        else:
            videosize = controlsize = self.mc.GetSize()
        try:   
            if not self.app.userprofile['mcattached']:
                framesize = self.mcframe.GetSize()
            frameaspect = float(framesize[0])/framesize[1]
            videoaspect = float(videosize[0])/videosize[1]
            if frameaspect < videoaspect:
                scale = float(framesize[0]) / videosize[0]
            else:
                scale = float(framesize[1]) / videosize[1]
            newvideosize = map(lambda x: int(scale*x), videosize)
            if USE_MPLAYER:
                self.mc.SetInitialSize(newvideosize)
                self.mc.SetSize(newvideosize)
            else:
                self.mc.SetInitialSize(newvideosize)
            
            if not self.app.userprofile['mcattached']:
                dx = framesize[0] - newvideosize[0]
                dy = framesize[1] - newvideosize[1]
                self.mc.SetPosition((dx/2, dy/2))
                
            self.sizer.Layout()
            if hasattr(self.mc, 'last_seek'):
                self.mc.Seek(self.mc.last_seek + .000001, 1)
        except ZeroDivisionError:
            if self.loaded: # Need to Play(), Pause() to fix DShow bug
                self.OnMediaLoaded()
        except ValueError: # happens when closing due to the config shelf being closed
            pass
        except Exception:
            pass
        except MplayerCtrl.NoMplayerRunning:
            pass
        

        
    def SetSensitivity(self, sensitivity=10.):
        self.unadjusted_sensitivity = sensitivity
        try:
            l = self.mc.Length() # mc might not be declared
            if not l or not type(l) in (int, float, long):
                raise AttributeError # and it might not have a length yet
        except AttributeError:   # If not, we'll try again later
            #if DEBUG: print "AttributeError while trying to SetSensitivity"
            wx.CallLater(500, self.SetSensitivity, sensitivity)
            return
        try:
            oldsens = self.sensitivity
        except: # self.sensitivity hasn't been defined; first execution
            oldsens = None
        self.sensitivity = sensitivity / l * 42275.
        if oldsens:
            if self.app.userprofile['feedbackmode'] == 'Smooth':
                self.smoothmovieoffset = self.sessionchange * (oldsens - self.sensitivity) * 30. + self.smoothmovieoffset
            else:
                self.movieoffset = self.percentchange * (oldsens - self.sensitivity) + self.movieoffset
        return True # success
        
    def OnSensitivityCtrl(self, evt=None):
        v = self.sens.GetValue()
        v = float(v)#*.5
        self.SetSensitivity(v)
        
    def Update(self, data, times):
        self.UpdateData(data, times)
        self.UpdateMovie()
        self.UpdateText()
        self.UpdatePlot(data, times)

    def CalcMoviePos(self):
        sensitivity = self.sensitivity if hasattr(self, 'sensitivity') else 1.0
        if self.app.userprofile['feedbackmode'] == 'Smooth':
            return self.sessionchange * sensitivity * 30. + self.smoothmovieoffset
        else:
            return self.percentchange * sensitivity + self.movieoffset



    def UpdateMovie(self, data=None, times=None):
        moviepos = self.CalcMoviePos()
        try:
            moviepos = (moviepos * self.mc.Length()) % (self.mc.Length())
        except ZeroDivisionError:
            pass
        InstrumentsPanel.UpdateMovie(self, data, times)
        #self.mc.Seek(moviepos)
        
        if not 'playpause' in self.app.cfg:
            self.app.cfg['playpause'] = False
        if self.app.cfg['playpause'] and not USE_MPLAYER:
            self.mc.Play()
            self.mc.Pause()

        if USE_THREADS:
            if not self.mcmutex.locked():
                self.mcmutex.acquire()
                thread.start_new(self.SeekMovieAndReleaseMutex, (moviepos,))
        else:
            self.mc.Seek(moviepos)
    def SeekMovieAndReleaseMutex(self, moviepos):
        self.mc.Seek(moviepos)
        self.mcmutex.release()
    

    def UpdatePlot(self, data, times):
        if USE_THREADS:
            if not self.plot.mutex.locked():
                self.plot.mutex.acquire()
                thread.start_new(self.plot.Update, (data, times))#self.plot.Update(data)
        else:
            self.plot.Update(data, times)

        
    def OnClose(self, evt=None):
        if hasattr(self, 'mcframe'):
            self.mcframe.Bind(wx.EVT_CLOSE, None)
        self.CloseMC()
    def CloseMC(self, evt=None):
        if hasattr(self, 'mc') and self.mc:
            #self.mc.Destroy()
            #self.sizer.Remove(self.mc)
            if USE_MPLAYER:
                self.mc.Quit()
            self.mc.Destroy()
        if hasattr(self, 'mcframe'):
            if not self.mcframe.IsMaximized():
                self.app.userprofile['mcframesize'] = self.mcframe.GetSize()
            self.app.userprofile['mcframemaximized'] = self.mcframe.IsMaximized()
            self.mcframe.Destroy()
            del self.mcframe, self.mcsizer

class BlindHEGFlight(HEGFlightInstrumentsPanel):
    def __init__(self, parent, id, app, blind=True, reverse=False, *args, **kwargs):
        HEGFlightInstrumentsPanel.__init__(self, parent, id, app, blind=blind, *args, **kwargs)

AvailableModes['HEGFlight'] = HEGFlightInstrumentsPanel
AvailableModes['BlindHEGFlight'] = BlindHEGFlight
