import wx, time, os, thread, sys, math, random, numpy, traceback
from __main__ import USE_MPLAYER, DEBUG, mediadir, opj, USE_THREADS, AvailableModes
import hegflight
import plotcanvas
import MplayerCtrl as MplayerCtrl
from instrumentspanel import InstrumentsPanel

mean = hegflight.mean

USE_OPENGL = False

class MplayerMovie(MplayerCtrl.MplayerCtrl):
    def __init__(self, *args, **kwargs):
        self.paused = False
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
        super(MplayerMovie, self).__init__(*args, **kwargs)
        self.force_keep_pause = True
        self.file_length = None

    def GetBestSize(self):
        if hasattr(self, 'res') and self.res:
            return self.res
        try:
            res = self.GetVideoResolution()
        except:
            #if DEBUG: print "Couldn't run self.GetVideoResolution()."
            res = None
        if res and not type(res) == float: # we often get video positions (floats) instead
            if type(res) == str:           # (MplayerCtrl bug)
                res = tuple(map(int, res.split(' x ')))
                #self._clear_queue()        # get rid of at least a few of the many spurious results

            self.res = res
            return res
        else:
            #print "GetBestSize failed"
            #time.sleep(.1) # GetBestSize fails for a few seconds after a video loads
            return None
    def Load(self, path, driver=None):
        if DEBUG: print "Load called"
        self.cached_length = None
        self.res = None
        if not self.process_alive:
            if USE_OPENGL:
                self.Start(media_file=path, mplayer_args=['-vo', 'gl,', '-loop', '0'])
            else:
                self.Start(media_file=path, mplayer_args=['-vf', 'eq2', '-loop', '0'])
        else:
            self.cached_length = None
            self.Loadfile(path)
        self.Osd(0)
        #self.Pause()
        self.FrameDrop(1)
        self.GetBestSize()
        return True

    def OnUpdate(self, data, times=[]):
        pass
#        delta = mean(data[-5:]) - mean(data[-50:])
#        sigma = numpy.std(data[-300:])
        
#        contrast = 1./(1+math.e**(-delta/sigma*5. - 0))*100. - 85.
#        self.Brightness(contrast, 1)
#        #if not random.randint(0, 30) and DEBUG: print contrast, delta, sigma
        
#        if   delta > -sigma/5:#-0.25:
#            if self.paused:
#                self.paused = False
#                self.pausing = False
#                self.Pause()
#                #if DEBUG: print "Now unpaused, %10.3f, %3.3f" % (time.time(), contrast)
#        elif delta < -sigma/4:
#            if not self.paused:
#                self.paused = True
#                self.pausing = True
#                self.Pause()
#                #if DEBUG: print "Now paused,   %10.3f" % time.time()

class HEGMovie(InstrumentsPanel):
    def __init__(self, *args, **kwargs):
        InstrumentsPanel.__init__(self, *args, **kwargs)
        self.SetDoubleBuffered(False) # needed for mplayer
        self.slider = wx.Slider(self, minValue=0, maxValue=1000000, size=(60,20), pos=(6,600))
        if DEBUG: print (self.mediarc[0]+30, self.mediarc[1])
        self.sizer.Add(self.slider, pos=(self.mediarc[0], self.mediarc[1]), span=(1,16), flag=wx.EXPAND)
        self.slider.Bind(wx.EVT_SCROLL_THUMBRELEASE, self.SliderSeek)
        self.slider.Bind(wx.EVT_SCROLL_CHANGED, self.SliderSeek)
        if 1:
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
        self.sizer.Layout()
        self.InitMediaCtrl(attach=self.app.userprofile['mcattached'])
        
    def SliderSeek(self, evt=None):
        if self.mc.paused:
            self.mc.Pause()
        newpos = self.slider.GetValue()
        self.mc.Seek(100.*newpos/1000000., 1)
        if self.mc.paused:
            self.mc.Pause()

    def UpdateMovie(self, data=[], times=[]):
        if len(data) < 20:
            return
    
        data, times = (numpy.array(data), numpy.array(times))
        delta = mean(data[-5:]) - mean(data[-50:])
        sigma = max(numpy.std(data[-300:]), .001)
        
        contrast = 1./(1+math.e**(-delta/sigma*5.))*100. - 85.
        zoom     = 1./(1+math.e**(-delta/sigma*2.))
        try:
            self.mc.Brightness(contrast, 1)
        except:
            if DEBUG: print "Couldn't change brightness."
        #self.mc.Panscan(zoom, 1)
        #print self.mc.GetProperty('panscan')
        #self.mc.ChangeRectangle(random.randint(0,1), random.randint(-5, 5))
        #self.OnResize(zoom=zoom)
        #if not random.randint(0, 30) and DEBUG: print contrast, delta, sigma
        
        trend = 1./(1+math.e**(-delta/sigma)) * 2 - 1
        if math.isnan(trend):
            print "trend was NaN in hegmovie.UpdateMovie. delta, sigma, trend = ", delta, sigma, trend
        r = int((1-max(-trend, 0))*64)
        g = int((1-math.fabs(trend))*64)
        b = int((1-max( trend, 0))*64)
        if trend < 0:
            b = 95 + int(-trend*160)
        else:
            r = 95 + int(trend*160)
        self.trendmeter.SetColor(wx.Colour(r,g,b))
        self.trendmeter.SetValue(trend)
        
        if   delta > -sigma/5:#-0.25:
            if self.mc.paused:
                self.mc.paused = False
                self.mc.pausing = False
                self.mc.Pause()
                print "Now unpaused, %10.3f, %3.3f" % (time.time(), contrast)
        elif delta < -sigma/4:
            if not self.mc.paused:
                self.mc.paused = True
                self.mc.pausing = True
                self.mc.Pause()
                print "Now paused,   %10.3f" % time.time()

        #self.mc.OnUpdate(data, times)
        if not hasattr(self, 'update_count'):
            self.update_count = 0
        self.update_count += 1
        if not self.update_count % 1:
            return;#t = self.mc.GetPercentPos()
            try:
                t = float(t)
                #if DEBUG: print "The time is now %3.1f" % t
                if not 'HEGMovie_moviePositions' in self.app.userprofile:
                    self.app.userprofile['HEGMovie_moviePositions'] = {}
                    #if DEBUG: print "Creating self.app.userprofile['HEGMovie_moviePositions']"

                # shelve won't notice if we update the dict-within-a-shelf,
                # so we have to replace it with an updated one
                positions = self.app.userprofile['HEGMovie_moviePositions']
                positions[self.app.userprofile['HEGMovie_video']] = t

                self.app.userprofile['HEGMovie_moviePositions'] = positions
                #if not self.mc.file_length:
                #l = self.mc.GetTimeLength()
                #    l = float(l)
                #    self.mc.file_length = l
                #else:
                #    l = self.mc.file_length
                #print t, l, t/l*1000000.
                self.slider.SetValue(int(t*10000.))
                
                
            except ValueError:
                print "Got bad value from MplayerCtrl.GetPercentPos(). It was %s." % (str(t))
                pass
            except TypeError:
                print "Got bad type from MplayerCtrl.GetPercentPos(). It was %s, %s." % (`type(t)`, str(t))
                pass
            except:
                traceback.print_exc()
                

    def InitMediaCtrl(self, attach=True):
        if DEBUG: print "InitMediaCtrl called"
        self.CloseMC()
        
        if not 'HEGMovie_video' in self.app.userprofile:
            self.OpenVideo()
            if DEBUG: print "No video found"
            return
        
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
        
        self.mc = MplayerMovie(parent=parent, id=-1)
        if self.LoadVideo(self.app.userprofile['HEGMovie_video']): # True if error
            try:
                self.LoadVideo(random.choice(self.mediafiles))
            except:
                if len(self.mediafiles) < 1:
                    return error("Couldn't find any video files in %s." % mediadir)

        
        if attach:
            self.sizer.Add(self.mc, (self.mediarc[0]+1, self.mediarc[1]), (29,16), flag=wx.ALIGN_CENTER_HORIZONTAL)
            self.Bind(wx.EVT_SIZE, self.OnResize)
        else:
            #self.mcsizer.Add(self.mc)
            self.mcframe.Bind(wx.EVT_SIZE, self.OnResize)
            self.mcframe.Bind(wx.EVT_CLOSE, lambda x: self.InitMediaCtrl(attach=True))
        self.app.userprofile['mcattached'] = attach
        self.mc.Bind(wx.EVT_CONTEXT_MENU, self.OnMCRightClick)

        if not self.mc.paused:
            self.mc.paused = True
            self.mc.pausing = True
            self.mc.Pause()

        self.sizer.Layout()

        
    def OnMCRightClick(self, evt=None):
        command = 'Attach'
        if self.app.userprofile['mcattached']: 
            command = 'Detach'
        
        menulist = [('Open &video file', self.OpenVideo),
                    ('Open &DVD disc',   self.OpenDisc),
                    (command, lambda x:  self.InitMediaCtrl(attach='toggle'))]
                    
        newmenu = wx.Menu()
        for item in menulist:
            newmenuitem = wx.MenuItem(parentMenu=newmenu, id=-1, text=item[0])
            self.Bind(wx.EVT_MENU, item[1], newmenuitem)
            newmenu.Append(newmenuitem)
        self.PopupMenu(newmenu, self.ScreenToClient(evt.GetPosition()))
        
    def OpenVideo(self, evt=None):
        if not 'HEGMovie_video' in self.app.userprofile:
            if "Python27" in sys.executable:
                defaultDir = os.path.dirname(os.path.abspath(__file__))[:-3] + "media"
            else:
                defaultDir = os.path.dirname(sys.executable) + "media"
        else:
            defaultDir = os.path.dirname(self.app.userprofile['HEGMovie_video'])
            try: os.path.listdir(defaultDir)
            except:
                if "Python27" in sys.executable: 
                    os.path.dirname(os.path.abspath(__file__))[:-3] + "media"
                else:
                    os.path.dirname(sys.executable)+"media"
        dialog = wx.FileDialog(parent=None, message='', defaultDir=defaultDir, 
                               wildcard='Video files (*.avi;*.mpg;*.wmv;*.mp4;*.mkv;*.m4v)|*.avi;*.mpg;*.wmv;*.mp4;*.mkv;*.m4v')
        res = dialog.ShowModal()
        if res == wx.ID_OK:
            self.loaded = False
            self.LoadVideo(dialog.GetPath())
            self.InitMediaCtrl(attach=self.app.userprofile['mcattached'])
    
    def OpenDisc(self, evt=None):
        if sys.platform == 'win32':
            import string
            from ctypes import windll

            #driveTypes = ['DRIVE_UNKNOWN', 'DRIVE_NO_ROOT_DIR', 'DRIVE_REMOVABLE', 
            #              'DRIVE_FIXED', 'DRIVE_REMOTE', 'DRIVE_CDROM', 'DRIVE_RAMDISK']
            drives = []
            bitmask = windll.kernel32.GetLogicalDrives()
            for letter in string.uppercase:
                if bitmask & 1:
                    try:
                        typeIndex = windll.kernel32.GetDriveTypeW(u"%s:\\"%letter)
                        if typeIndex == 5: # 'DRIVE_CDROM'
                            drives.append(letter + ':')
                    except: pass
                bitmask >>= 1
                
            if not drives:
                 return error("No DVD drives could be found.")
        elif sys.platform == 'darwin':
            return error("This feature has not yet been implemented for Mac OS X.")
        else:
            return error("This feature has not yet been implemented for whatever operating system you're using.")
        
        dlg = wx.SingleChoiceDialog(self, message="Message", caption='Caption', choices=drives)
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            drive = drives[dlg.GetSelection()]
            self.LoadVideo('dvd://' + drive + '\\')
            #print "I would now open drive %s" % drive
        
            

    def LoadVideo(self, path):
        self.app.userprofile['HEGMovie_video'] = path
        try: 
            os.path.listdir(path)
            print path
        except:
            if "Python27" in sys.executable:
                path = os.path.dirname(os.path.abspath(__file__))[:-3] + "media" 
            else:
                path = os.path.dirname(sys.executable) + "\\media"
            path = path + "\\" + os.listdir(path)[0]
            print path
        if not hasattr(self, 'mc'):
            self.InitMediaCtrl()
        if DEBUG: print "Loading %s" % path
        if not os.path.exists(path) and os.path.exists(opj(mediadir, path)):
            path = opj(mediadir, path)
        elif not os.path.exists(path) and not (path.startswith('dvd') and '://' in path):
            if DEBUG: print "That path doesn't seem to exist."
            return True
        self.loaded = False
        if DEBUG: print "Trying to load %s" % path
        if not self.mc.Load(path):
            wx.MessageBox("Unable to load %s: Unsupported format?" % path, "ERROR", wx.ICON_ERROR | wx.OK)
            return True
        
        if 'HEGMovie_moviePositions' in self.app.userprofile and \
            self.app.userprofile['HEGMovie_video'] in \
            self.app.userprofile['HEGMovie_moviePositions']:
            t = self.app.userprofile['HEGMovie_moviePositions']\
                      [self.app.userprofile['HEGMovie_video']]
            if t > 95.: t = 0
            if DEBUG: print "Seeking to %3.1f" % t
            #self.mc.Seek(t, 1)
        
        self.mc.SetInitialSize()
        
        if not self.mc.paused:
            self.mc.paused = True
            self.mc.pausing = True
            self.mc.Pause()
        
        wx.CallLater(500, self.OnResize, self)
        wx.CallLater(1500, self.OnResize, self)
        wx.CallLater(4500, self.OnResize, self)
        return False
 
    def OnMediaLoaded(self, evt=None):
        #print "OnMediaLoaded called"
        self.sizer.Layout()
        size = self.mc.GetBestSize()
        if size and size[0]:
            self.OnResize(evt)
            self.loaded = True
        
    def OnResize(self, evt=None, zoom=1.):
        InstrumentsPanel.OnResize(self, evt)
        self.mc.SetInitialSize()
        #print self.mc.GetBestSize(), self.mc.Length()
        totalframesize = self.parent.GetBestSize()
        #self.plot.SetMinSize((totalframesize[0], totalframesize[1]/5))    
        framesize = totalframesize[0] - 200, totalframesize[1]*4/5 - 50
        if USE_MPLAYER:
            for i in range(10):
                videosize = self.mc.GetBestSize()
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
            scale = scale * zoom
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
        except ZeroDivisionError:
            if self.loaded: # Need to Play(), Pause() to fix DShow bug
                self.OnMediaLoaded()
        except ValueError: # happens when closing due to the config shelf being closed
            pass

    def Destroy(self):
        self.OnClose()
        wx.Panel.Destroy(self)
        
    def OnClose(self, evt=None):
        if hasattr(self, 'mcframe'):
            self.mcframe.Bind(wx.EVT_CLOSE, None)
        self.CloseMC()
        
    
    def CloseMC(self, evt=None):
        if hasattr(self, 'mc') and self.mc:
            self.sizer.Remove(self.mc)
            self.mc.Quit()
            self.mc.Destroy()
        if hasattr(self, 'mcframe'):
            if not self.mcframe.IsMaximized():
                self.app.userprofile['mcframesize'] = self.mcframe.GetSize()
            self.app.userprofile['mcframemaximized'] = self.mcframe.IsMaximized()
            self.mcframe.Destroy()
            del self.mcframe, self.mcsizer
            
     
    def OnPause(self, evt=None):
        InstrumentsPanel.OnPause(self, evt)
        if not self.mc.paused:
            self.mc.paused = True
            self.mc.pausing = True
            self.mc.Pause()  
        
    def OnStop(self, evt=None):
        InstrumentsPanel.OnStop(self, evt)
        if not self.mc.paused:
            self.mc.paused = True
            self.mc.pausing = True
            self.mc.Pause()  
        
        
AvailableModes['HEGMovie (beta)'] = HEGMovie
