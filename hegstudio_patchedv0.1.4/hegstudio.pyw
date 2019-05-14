#!/usr/bin/env python

################################################################################
# HEGStudio/HEGFlight
# An application suite for hemoencephalography neurofeedback
# 
# Copyright 2010-2012, Jonathan Toomim (jtoomim@jtoomim.org)
# This software is available under the terms of the GNU Lesser Public License,
# version 3.0. For a copy of this license, see the file LGPL.txt, or visit 
# http://www.gnu.org/licenses/lgpl-3.0.txt.
#
# Want to adapt this software for your project? Want to make your biofeedback
# hardware supported by this software? Please do! Email me, and I might even
# help.

# Patched by Joshua Brewster (0.4.9) fixed hegmovie.py, updated function calls, HEGduino support
################################################################################

__VERSION__ = version = '0.4.9'

import wx
import numpy
import wx.lib.plot
import lib.plot
import wx.lib.embeddedimage
wx.lib.plot = lib.plot
import wx.lib.buttons
import numpy, thread, shelve, platform, traceback, subprocess

import dbhash # win32 only? needed for py2exe
import os, sys, time, random
opj = os.path.join # The full name is just so bulky and annoying

DEBUG = '--debug' in sys.argv
USE_THREADS = '--thread' in sys.argv
USE_MPLAYER = '--mplayer' in sys.argv or (not '--nomplayer' in sys.argv and not sys.platform == 'darwin')
USE_HEGINPUT = '--heginput' in sys.argv
SLOW_PC = '--slow-pc' in sys.argv

if DEBUG:
    import wx.py

if USE_HEGINPUT:
    try:
        from lib import cheginput as heginput
        if DEBUG: print "Using cheginput"
    except:
        from lib import heginput
else:
    from lib import parsers

heg_wildcard = 'HEG files (*.heg;*.pnb;*.pea;*.heg.zip)|*.heg;*.pnb;*.pea;*.heg.zip'
#heg_wildcard = 'HEG files (*.pnb;*.pea;*.heg;*.heg.zip)|*.pnb;*.pea;*.heg;*.heg.zip'

if sys.platform == 'darwin':
    mediadir    = os.path.expanduser(u'~/Movies/HEG videos')
    sessionsdir = os.path.expanduser(u'~/Documents/HEG sessions')
    settingsdir = os.path.expanduser(u'~/Library/Application Support/HEGStudio')
else:
    mediadir    = u'media'
    #sessionsdir = opj(os.path.expanduser('~/Documents/HEG sessions'))
    if sys.platform == 'win32':
        import _winreg as winreg

        path = 'Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders'
        try:
            key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, path)
            sessionsdir = opj(winreg.QueryValueEx(key, 'Personal')[0], 'HEG Sessions')
            if DEBUG:
                print sessionsdir
        except WindowsError:
            if DEBUG:
                print "Couldn't find 'My Documents' (or equivalent) folder. Falling back to hardcoded path."
                sessionsdir = opj(os.path.expanduser('~/Documents/HEG sessions'))
        try:
            appdata = unicode(os.environ['APPDATA'])
        except:
            appdata = u'.'
        settingsdir = opj(appdata, u'HEGStudio')
    else:
        settingsdir = os.path.expanduser(u'~/.HEGStudio')
try:
    settingsdir = settingsdir.encode('ascii')
except:
    settingsdir = settingsdir.encode('utf-8')
    if DEBUG: print "using UTF-8 for settingsdir"
settingsdir = None # fix this later using wx.StandardPaths.Get() after wx.App() is created
if DEBUG: print "settingsdir is " , settingsdir

fauxdir = opj(sessionsdir, u'faux')

try:    fauxsessions = [opj(fauxdir, fn) for fn in os.listdir(fauxdir) 
                        if fn.lower()[-4:] in ('.pnb', '.pea', '.heg')]
except: fauxsessions = []

def error(message):
    try:
        dlg = wx.MessageDialog(self, message, style=wx.OK)
    except:
        dlg = wx.MessageDialog(None, message, style=wx.OK)
    dlg.ShowModal()



AvailableModes = {} # this will be added to by the modules we import

from lib import review, plotcanvas, waterfall, hegflight, hegmovie, simplegraphs
#AvailableModes{'HEGFlight':hegflight.HEGFlightInstrumentsPanel, 
#         'HEGMovie (beta)':hegmovie.HEGMovie,
#         'SimpleGraphs (beta)':simplegraphs.SimpleGraphsPanel}


class Console(wx.ScrolledWindow):
    def __init__(self, *args, **kwargs):
        wx.ScrolledWindow.__init__(self, *args, **kwargs)
        self.bigframe = None
        self.messages = []
        self.statictexts = []
        
    def AddMessage(self, message, priority=0, duration=4.):
        self.messages.append([message, priority, duration. time.time()])
        self.statictexts.append([wx.StaticText(parent=self, label=message), priority, duration, time.time()])
        

    


class NewSessionForm(wx.Dialog):

     #"%s - %04i.%02i.%02i %02ih%02im - " % ((self.user,) + time.localtime()[0:5])
    
    def __init__(self, app, *args, **kwargs):
        wx.Dialog.__init__(self, title="New Session",style= wx.DEFAULT_DIALOG_STYLE| wx.RESIZE_BORDER,size=(600,450), *args, **kwargs)
        self.app = app
        user = app.user
        self.profile = profile = app.userprofile
        

        if not 'timestamp_in_filename' in self.profile:
            self.profile['timestamp_in_filename'] = True

        self.defaultFile = "%s" % (self.app.user)
        if self.profile['timestamp_in_filename']:
            self.defaultFile += " - %04i.%02i.%02i %02ih%02im" % tuple(time.localtime()[0:5])
        self.defaultFile += ".heg"
        
        folders = [''] + [folder for folder in os.listdir(self.app.usersessionsdir) if os.path.isdir(opj(self.app.usersessionsdir, folder))]

        self.filenameTyped=False
        #EVT_TEXT
        self.topsizer = wx.BoxSizer(wx.VERTICAL)
        self.buttonsizer = wx.BoxSizer(wx.HORIZONTAL)
        self.fnamesizer = wx.GridBagSizer(vgap=2, hgap=20)
        self.grid = wx.GridBagSizer(vgap=2, hgap=20)

        # 'Subject', 'Training site', and 'File description must be the 0th, 
        # 1st, and 2nd element of this list
        self.formformat = [{'label':'Subject', 'key':'subject', 'format':'string', 
                       'default':app.user, 'info':'Subject identification','multiline':False},
                      {'label':'Save folder', 'key':'folder', 'format':'category', 
                       'default':'', 'options':folders, 'fixed':False,
                       'info':'Subfolder of HEG Sessions/%s into which to save the file' % app.user,'multiline':False},
                      {'label':'Training site', 'key':'site', 'format':'category', 
                       'default':'', 'options':profile['sites'], 'fixed':False,
                       'info':'Position where the HEG headband is placed (e.g. center, or Fpz)','multiline':False},
                      {'label':'File description', 'key':'fileinfo', 'format':'string',
                       'default':'', 'info':'Anything entered here will be added\n'
                                            'to the default save file name.','multiline':True},
                      {'label':'Time limit (seconds)', 'key':'timelimit', 'format':'int', 
                       'default':0, 'min':0, 'max':360000,
                       'info':'Time limit for this session. Zero means no limit.','multiline':False},
                      {'label':'Notes', 'key':'notes', 'format':'textbox',
                       'default':'','multiline':True}]
        
        self.formcontrols = []
        row = 0

        for item in self.formformat:
            label = wx.StaticText(self, label=item['label'])
            style = 0
            
            if item['multiline']==True:
                style =wx.TE_MULTILINE
            
            if item['format'] in ('string', 'textbox'):              
               control = wx.TextCtrl(self, value=item['default'], style=style)
               control.Bind (wx.EVT_TEXT, self.OnTyped)
               
            elif item['format'] == 'int':
                control = wx.SpinCtrl(self, initial=item['default'], 
                                      min=item['min'], max=item['max'])
            
            elif item['format'] == 'category' and item['fixed']:
                control = wx.Choice(self, choices=item['options'])
                #control.Bind(wx.EVT_CHOICE, self.OnTyped)
                control.Bind(wx.EVT_CHOICE, self.OnTyped)
                control.Bind(wx.EVT_CHAR, self.OnTyped)
                
            elif item['format'] == 'category':
                control = wx.ComboBox(self, value=item['default'], choices=item['options'])
                control.Bind (wx.EVT_COMBOBOX, self.OnTyped)
                
            if 'info' in item:
                tooltip = wx.ToolTip(item['info'])
                label.SetToolTip(tooltip)
                control.SetToolTip(tooltip)
            
            self.grid.Add(label, (row, 0),(1,1),wx.ALIGN_LEFT|wx.ALL|wx.GROW,5)
            width = 1
            if item['multiline']==True :
                width = 10

            if item['format'] == 'textbox':
                self.grid.Add(control, (row, 1), (8, width), wx.ALL|wx.GROW )
                row += 7
            else:
                self.grid.Add(control, (row, 1),(1,width),wx.ALL|wx.GROW)
                row += 1
            self.formcontrols.append([item, label, control])       

        self.time_text  = wx.StaticText(self, label="Timestamp")
        self.time_check = wx.CheckBox(self)
        self.time_check.SetValue(self.profile['timestamp_in_filename'])
        self.time_check.Bind(wx.EVT_CHECKBOX, self.OnCheck)
        self.fnamesizer.Add(self.time_text,  (row, 0), (1,1), wx.ALIGN_LEFT|wx.ALL|wx.GROW, 5)
        self.fnamesizer.Add(self.time_check, (row, 1), (1, 10), wx.ALL|wx.GROW )
        row += 1

        self.fname_cntrl = wx.TextCtrl(self,value=self.defaultFile)
        self.fname_label = wx.StaticText(self, label="File Name")
        self.fname_exists= wx.StaticText(self, label="")
        self.button_1 = wx.Button(self, wx.ID_OK, "")
        self.button_2 = wx.Button(self, wx.ID_CANCEL, "")
        
        self.fnamesizer.Add(self.fname_label,  (row, 0), (1,1), wx.ALIGN_LEFT|wx.ALL|wx.GROW, 5)
        self.fnamesizer.Add(self.fname_cntrl,  (row, 1), (1,9), wx.ALL|wx.GROW )
        self.fnamesizer.Add(self.fname_exists, (row,10), (1,1) )

        self.fname_cntrl.Bind (wx.EVT_CHAR, self.OnFilenameTyped )
        self.fname_cntrl.Bind (wx.EVT_TEXT, self.CheckCollision )

        
        self.buttonsizer.Add(self.button_1, 0, 0, 0)
        self.buttonsizer.Add(self.button_2, 0, wx.LEFT, 10)   

        self.SetSizer(self.topsizer)
        self.topsizer.Add(self.grid,1,wx.EXPAND)
        self.topsizer.Add(self.fnamesizer, 0, wx.ALL | wx.ALIGN_LEFT, 5)
        
        self.topsizer.Add(self.buttonsizer, 0, wx.ALL | wx.ALIGN_CENTRE|wx.ALIGN_BOTTOM, 5)
        
        # subject, location, timelimit, protocol/strategy?, 
        self.OnTyped()
    
    def OnCheck(self, evt=None):
        self.profile['timestamp_in_filename'] = self.time_check.GetValue()
        self.filenameTyped=False
        self.OnTyped()

    def OnTyped(self, evt=None):
        vals = [self.formcontrols[i][2].GetValue() for i in range(4)]
        folder = vals.pop(1)
        newvals = filter(lambda x: bool(x), vals) # no extra ' - 's
        if not vals[0]:
            newvals.insert(0, 'Anonymous')
        if self.profile['timestamp_in_filename']:
            newvals.insert(1, "%04i.%02i.%02i %02ih%02im" % time.localtime()[0:5])
        text = ' - '.join(newvals) + '.heg'
        if folder:
            text = opj(folder, text)
        self.defaultFile = text
        if not self.filenameTyped:
            self.fname_cntrl.SetValue(text)
        else:
            print self.filenameTyped
        
        self.CheckCollision()
    
    def CheckCollision(self, evt=None):
        if os.path.exists(opj(self.app.usersessionsdir, self.defaultFile)):
            self.fname_exists.SetLabel("File exists!")
            self.button_1.Enable(False)
        else:
            self.fname_exists.SetLabel("")
            self.button_1.Enable(True)

            
        #evt.Skip()   
 
    def OnFilenameTyped(self, evt=None):
        self.filenameTyped=True  
        self.defaultFile = self.fname_cntrl.GetValue()
        evt.Skip()

    def GetConfig(self):
        config = {}
        for format, control in zip(self.formformat, self.formcontrols):
            config[format['key']] = control[2].GetValue()
        config['original_filename'] = self.defaultFile
        return config
    
    


class MyApp(wx.App):
    def OnInit(self, evt=None):
        global settingsdir
        global sessionsdir
        
        self.SetAppName("HEGStudio")
        
        self.hegduino = False

        sp = wx.StandardPaths.Get()
        settingsdir = sp.GetUserDataDir()
        sessionsdir = opj(sp.GetDocumentsDir(), u'HEGStudio')
        
        self.cfg = self.LoadSettings()
        if 'lastuser' in self.cfg:
            self.user = self.cfg['lastuser']
        else:
            self.user = 'DefaultUser'

        if USE_MPLAYER:
            self.mpc_quits = []

        self.frame = wx.Frame(None, -1, "HEGStudio", size=(1024,600))
        self.frame.Bind(wx.EVT_CLOSE, self.OnExit)
        
        menutree = [['File', 
                     [['New user', self.NewUser],
                      ['Switch users', self.SwitchUsers],
                      #['Open &video', self.OpenVideo], 
                      ['&New session', self.NewSession],
                      ['&Save session', self.SaveSession],
                      #['Save session as', self.SaveSessionAs],
                      ['Replay a session', self.OpenSession],
                      ['Export data', self.ExportSession],
                      #['Double-blind session', self.RunDoubleBlind],
                      ['Double-blind random direction session', self.RunDoubleBlindDirection],
                      ['Review sessions', self.ReviewSessions],
                      ['E&xit', self.OnExit] ]],
                    ['Mode',
                     [['Change mode', self.ModeDialog]]]]
        if DEBUG:
            menutree += \
                   [['Debug', 
                     [['PyCrust', self.OpenCrust],
                      ['PyFilling', self.OpenFilling]]]]
            self.crustframes = []
            self.crusts = []
        
        self.menubar = wx.MenuBar()
        self.menus = {}
        for menu in menutree:
            newmenu = self.menus[menu[0]] = wx.Menu()
            for item in menu[1]:
                newmenuitem = wx.MenuItem(parentMenu=newmenu, id=-1, text=item[0])
                self.Bind(wx.EVT_MENU, item[1], newmenuitem)
                newmenu.Append(newmenuitem)
            self.menubar.Append(newmenu, menu[0])
        
        if not DEBUG: self.frame.Maximize()
        self.frame.SetMenuBar(self.menubar)
        self.frame.Show(1)
        
        self.SetUser(self.user)
        #self.panel = InstrumentsPanel(self.frame, -1, app=self)

        if hasattr(self, 'panel'):
            self.panel.OnResize()
        
        comport = None
        for arg in sys.argv:
            if arg.startswith('--com='):
                comport = arg.split('=')[1]
                if comport.isdigit():
                    comport = int(comport)
        if not comport and '--fake' in sys.argv: 
            comport = random.choice(fauxsessions)
            if DEBUG: print "using fake data:", comport
    
        self.waterfall = None
        
        self.paused = False
        
        self.InitHEG(comport=comport)

        if '--wport' in sys.argv:
            self.InitWaterfall()
        
        self.lastupdate = time.time()
        self.lastupdatelen = {}
        self.updatetimer = wx.Timer()
        self.updatetimer.Bind(wx.EVT_TIMER, self.OnUpdate)
        
        updatedelay = 33 # in ms
        if sys.platform == 'darwin':
            updatedelay = 13
        
        if SLOW_PC:
            updatedelay *= 2
        
            # DirecShow and WMP10 backends appear to not display a frame if we 
            # try to seek to a new frame before the last seek has been completed,
            # so we need to not seek too often to prevent major stuttering.
            # This only seems to be an issue on fast computers if a more complex
            # codec than MJPEG is used.
        self.updatetimer.Start(updatedelay)
        
        self.timers = [self.updatetimer]
               
        if not USE_MPLAYER and platform.platform().lower().startswith('windows-7'):
            error("You appear to be using Windows 7. This program does not work well with Windows Media Player 12, which " +\
                  "is bundled with Windows 7. It is strongly recommended that you run this program on Mac OS X, Windows XP, or " +\
                  "Windows Vista instead. If you don't, the video will be very jerky, with a lot of unnecessary " +\
                  "back-and-forth movement. If your computer is reasonably fast, you could run Windows XP in a virtual " +\
                  "machine on top of Windows 7.")
            if not 'playpause' in self.cfg: self.cfg['playpause'] = True
        else:
            if not 'playpause' in self.cfg: self.cfg['playpause'] = False
        return True

    def SetMode(self, mode):
        self.userprofile['mode'] = mode
       
        if hasattr(self, 'panel'):
            self.panel.Destroy()
        try:
            self.panel = AvailableModes[mode](self.frame, -1, app=self, size=self.frame.GetSize())
            self.frame.SetTitle("HEGStudio %s - %s - %s" % (version, self.user, self.userprofile['mode']))
        except:
            error("Unable to set mode %s. \n\nFull text of error:\n\n%s" % (mode, traceback.format_exc()))
            try:
                self.panel.Destroy()
            except:
                pass
        
    def ModeDialog(self, evt=None):
        choices = AvailableModes.keys()
        index = 0
        
        #dlg = wx.MultiChoiceDialog(parent=None, message="Choose a mode", 
        dlg = wx.SingleChoiceDialog(parent=None, message="Choose a mode", 
                                   caption="Choose which training mode you want to use",
                                   choices=choices)
        if self.userprofile['mode'] in choices:
            index = dlg.SetSelection(choices.index(self.userprofile['mode']))
        
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            #choice = dlg.GetSelections()
            #if len(choice) > 1:
            #    error('You can only choose one mode at a time right now.')
            #elif len(choice) == 1:
            #    choice = choice[0]
            #    self.SetMode(choices[choice])
            choice = dlg.GetSelection()
            self.SetMode(choices[choice])

    def ListUsers(self):
        users = [user for user in os.listdir(sessionsdir) if not user.lower() == 'faux' 
                 and os.path.isdir(opj(sessionsdir, user)) and not user.startswith('.')]
        return users

    def SwitchUsers(self, evt=None):
        users = self.ListUsers()
        dlg = wx.SingleChoiceDialog(parent=self.frame, message="Choose a user", caption="Choose a user", choices=users)
        dlg.SetSelection(users.index(self.user))
        res = dlg.ShowModal()
        if DEBUG:
            print type(dlg.GetStringSelection()), dlg.GetStringSelection().encode('utf-8')
        if res == wx.ID_OK:
            self.SetUser(dlg.GetStringSelection())
    
    def SetUser(self, newuser):
        try:
            isreload = 'userprofile' in self.__dict__
            if isreload:
                self.userprofile.sync()
                self.userprofile.close()
            self.user = newuser
            self.cfg['lastuser'] = self.user
            self.frame.SetTitle("HEGStudio %s - %s" % (version, self.user))
            
            print 'line 476 SettingsDir is ', settingsdir.encode('utf-8'), os.path.exists(settingsdir)
            if not os.path.exists(settingsdir): os.mkdir(settingsdir)
            
            
            #workaround for bug with bsddb not supporting non-ascii filenames
            
            path = self.user + u'-profile.db'
            
            try:
                path = path.encode('ascii')
            except:
                path = path.encode('utf-8').encode('base64').strip('\n')
            
            if DEBUG: print "Line 489 path = ", path
            
            oldpath = os.getcwdu()    
            os.chdir(settingsdir)
            sh = shelve.open(path)
            
            os.chdir(oldpath.encode('utf-8'))
                        
            self.userprofile = sh
            
            
            
            defaults = {'feedbackmode':'Responsive', 'sensitivity':5, 'video':'HEGFlight - Lakes Mountains.avi',
                        'mode':'HEGFlight', 'mcattached':True, 'mcframesize':(720, 400), 'mcframemaximized':False,
                        'sites':['', 'left', 'right', 'center'], 'mode':'HEGFlight'}
            for k,v in defaults.items():
                if not k in self.userprofile:
                    self.userprofile[k] = v
            #if USE_MPLAYER:
                #for f in self.mpc_quits:
                #    f()
                #self.mpc_quits = []
            if hasattr(self, 'panel'):
                self.panel.Destroy()
                del self.panel
            #self.panel = hegflight.HEGFlightInstrumentsPanel(self.frame, -1, app=self, size=self.frame.GetSize())
            #self.panel = hegmovie.HEGMovie(self.frame, -1, app=self, size=self.frame.GetSize())
            self.SetMode(self.userprofile['mode'])
            #this hack seems to no longer be necessary
            #if USE_MPLAYER:
            #    self.mpc_quits.append(self.panel.mc.Quit)
            
            if not os.path.exists(sessionsdir): os.mkdir(sessionsdir)
            self.usersessionsdir = usersessionsdir = opj(sessionsdir, self.user)
            if not os.path.exists(usersessionsdir): os.mkdir(usersessionsdir)
        except:
            error("Error switching to user profile %s. \n\nFull text of error:\n\n%s" % (newuser, traceback.format_exc()))

    def NewUser(self, evt=None):
        dlg = wx.TextEntryDialog(parent=self.frame, message='Enter the new username', caption='New user')
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            newuser = dlg.GetValue()
            newuser = newuser.strip()
            self.user = newuser
            if self.user in self.ListUsers():
                dlg = wx.MessageDialog(parent=self.frame, message="The user profile %s already exists. Selecting it now." % self.user,
                                       style=wx.OK|wx.CENTRE)
            else:
                if not os.path.exists(sessionsdir): os.mkdir(sessionsdir)
                if not os.path.exists(opj(sessionsdir, self.user)): os.mkdir(opj(sessionsdir, self.user))
                #self.userprofile = shelve.open(opj(settingsdir, self.user + '-profile.db'))
                self.SetUser(self.user)

    def ReviewSessions(self, evt=None):
        if 0:
            dialog = wx.DirDialog(parent=None,
                                  defaultPath=opj(sessionsdir, self.user))
            dialog.SetPath(opj(sessionsdir, self.user))
            dialog.SetMessage("Pick a file, any file.")
            dialog.SetTitle("Choose a directory with files to review.")
            res = dialog.ShowModal()
            if res == wx.ID_OK:
                path = dialog.GetPath()
        else:
            dlg = wx.FileDialog(parent=self.frame, 
                                message='Pick a file, any file. All HEG sessions in this directory will be graphed.', 
                                defaultDir=opj(sessionsdir, self.user),
                                wildcard=heg_wildcard,
                                style=wx.FD_OPEN)
            res = dlg.ShowModal()
            if res == wx.ID_OK:
                path = dlg.GetDirectory()

        if res == wx.ID_OK:self.review = review.ReviewFrame(path=path, parent=None)
            
    def NewSession(self, evt=None):
        nsf = NewSessionForm(app=self, parent=None)
        res = nsf.ShowModal()
        if res == wx.ID_OK:
            #print self.heg, self.heg.running
            #oldheg = self.heg
            if self.heg:
                self.panel.OnStop()#self.CloseHEG()
                time.sleep(0.5)
#            print self.heg, oldheg, oldheg.running
#            try:
#                oldheg.mgr.join(timeout=1.)
#                print self.heg, oldheg, oldheg.running

#            except:
#                if DEBUG: traceback.print_exc()
            config = nsf.GetConfig()
            if DEBUG: print config
            self.panel.OnPlay()#self.InitHEG()
            self.heg.join(timeout=3.) # give the protocol detector 3 seconds
            self.SaveSession(savefile=opj(self.usersessionsdir, config['original_filename']))
            hegconfig = self.heg.mgr.metadata#self.heg.GetMetadata()
            if DEBUG: print hegconfig, self.heg.metadata
            hegconfig.update(config)
        
    
    def SaveSession(self, evt=None, savefile=None):
        if not os.path.exists(sessionsdir): os.mkdir(sessionsdir)
        usersessionsdir = self.usersessionsdir
        if not os.path.exists(usersessionsdir): os.mkdir(usersessionsdir)
        
        if savefile == None:
            dlg = wx.FileDialog(parent=self.frame, 
                                message='Enter a filename to save HEG data into.', 
                                defaultFile="%s - %04i.%02i.%02i %02ih%02im - " % ((self.user,) + time.localtime()[0:5]),
                                defaultDir=usersessionsdir, 
                                wildcard=heg_wildcard,
                                style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
            res = dlg.ShowModal()
            if res == wx.ID_CANCEL:
                return
            elif res == wx.ID_OK:
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
        else: # savefile != None
            if os.path.basename(savefile) == savefile:
                savefile = opj(usersessionsdir, savefile)
            sessionfile = savefile
        if DEBUG: print 'self.heg.LogRawToFile(%s)' % sessionfile
        if self.heg: # FIXME: what if not self.heg?
            if USE_HEGINPUT:
                self.heg.logRawToFile(sessionfile) 
            else:
                self.heg.LogRawToFile(sessionfile) 
        return True

    def ExportSession(self, evt=None):
        dlg = wx.FileDialog(parent=self.frame, message='Select a source session to export',
                            defaultDir=opj(sessionsdir, self.user), 
                            wildcard=heg_wildcard,
                            style=wx.FD_OPEN)
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            src = dlg.GetPath()
            dlg.Destroy()
        else: 
            dlg.Destroy()
            return
        dlg = wx.FileDialog(parent=self.frame, message='Choose a destination file for exportation', 
                            defaultDir=opj(sessionsdir, self.user), wildcard='CSV files (*.csv)|*.csv',
                            style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT)
        res = dlg.ShowModal()
        if res == wx.ID_OK:
            dst = dlg.GetPath()
            parsers.export_csv(src, dst)
        #error("Not yet implemented.")

    def RunDoubleBlind(self, evt=None):
        if random.randint(0, 1):
            comport = random.choice(fauxsessions)
            print "Pseudo-feedback"
        else:
            comport = None
            print "True feedback"
        self.CloseHEG()
        self.panel.Clear(reinit=False)
        self.InitHEG(comport=comport)
        pass

    def RunDoubleBlindDirection(self, evt=None):
        controlgroup = bool(random.randint(0, 1))
        #self.panel = 
    
    def OpenSession(self, evt=None):
        dialog = wx.FileDialog(parent=self.frame, message='', defaultDir=opj(sessionsdir, self.user), 
                               wildcard=heg_wildcard)
        res = dialog.ShowModal()
        if res == wx.ID_OK:
            t1 = time.time()
            #newframe = wx.Frame(parent=None) # Testing the PreviewPanels
            #newpreview = review.PreviewPanel(path=dialog.GetPath(), parent=newframe)
            #newframe.SetSize((700, 200))
            #newframe.Show()
            self.CloseHEG()
            t2 = time.time()
            #print "Previewing session %s took %2.3f seconds." % (dialog.GetPath(), t2-t1)
            self.panel.Clear(reinit=False)
            self.InitHEG(comport=dialog.GetPath())
        
    def AutosaveHEG(self, evt=None):
        comport = self.heg.port
        if (not hasattr(comport, 'split')) or comport.startswith('/dev/'):
            #if DEBUG: print "trying to autosave"
            ct = time.localtime()
            if not os.path.exists(sessionsdir): os.mkdir(sessionsdir)
            if not os.path.exists(opj(sessionsdir, self.user)): os.mkdir(opj(sessionsdir, self.user))            
            rawlogdir = opj(sessionsdir, self.user, u'autosaves')
            if not os.path.exists(rawlogdir): os.mkdir(rawlogdir)
            rawlog = u"%s - autosave - %04i.%02i.%02i %02ih%02im" % ((self.user,) + ct[0:5])
            rawlog = opj(rawlogdir, rawlog)
            if DEBUG:
                print u"Autosaving from port " + unicode(comport),
                print " to file:\n" , rawlog.encode('utf-8')
            if USE_HEGINPUT:
                self.heg.logRawToFile(rawlog.encode('utf-8'))
            else:
                if DEBUG: print "self.heg.LogRawToFile"
                self.heg.LogRawToFile(rawlog)

    def AskUserForPort(self, ports):
        pcd = PortChooseDialog(parent=self.frame, port_options=ports)
        res = pcd.ShowModal()
        if res == wx.ID_OK:
            choice = pcd.GetStringSelection()
            return choice

    def EnumeratePorts(self):
        if USE_HEGINPUT:
            ports = heginput.enumerate_serial_ports()
        else:
            ports = parsers.enumerate_serial_ports()
            print ports
        if DEBUG: print ports
        if sys.platform == 'win32':
            portsTemp = [port[0] for port in ports if 'Silabser' in port[1]]
            if str(portsTemp).find('COM') == -1:
                ports = [port[0] for port in ports if 'VCP' in port[1] or 'USBSER' in port[1]]
            #if str(ports).find('Silabser') != -1:
                #self.hegduino = true
            else:
                ports = portsTemp
                self.hegduino = True
            print 'Device Ports: ', ports
        else:
            ports = [port[0] for port in ports]
        return ports


    def InitHEG(self, evt=None, comport=None):     
        self.paused_time = 0.   
        if self.paused: self.pause_start = time.time()
        self.CloseHEG()
        if USE_HEGINPUT:
            self.heg = heginput.hegstream()
        if not comport:
            ports = self.EnumeratePorts()
            if DEBUG: print ports
            if not comport:
                if len(ports) == 0:
                    error("Couldn't find the wireless dongle. Is it plugged in? Is the driver properly installed?")
                    return "no port"
                elif len(ports) > 1:
                    comport = self.AskUserForPort(ports)
                    #print "WARNING: Found %i possible serial ports. Trying the first one, %s." % (len(ports), ports[0])
                else: comport = ports[0]
            if DEBUG:
                print "Trying to use comport",  comport
            #if sys.platform == 'win32':
            #    if comport.lower().startswith('com'):
            #        comport = int(comport[3:])
        try:
            if USE_HEGINPUT:
                self.heg.initialize(port=comport, pause=self.paused)
            elif self.hegduino == True:
                print 'HEGduino found on port ', comport
                self.heg = parsers.AutoHEG(port=comport, pause=self.paused, baudrate=115200, timeout=2) # Run ProtocolDetector
                self.heg.start() # Run Managers
            else:
                print 'Device found on port ', comport
                self.heg = parsers.AutoHEG(port=comport, pause=self.paused, baudrate=38400, timeout=2)
                self.heg.start()
            #if not '--noautosave' in sys.argv:
             #   self.AutosaveHEG()
        except ValueError:#lib.heginput.DataError: # FIXME: what about parsers.py equivalent?
            error("Serial port found, but there is no data is coming from it. Perhaps " + \
                  "it's the wrong port, or the HEG system isn't on.")
            if DEBUG: traceback.print_exc()
            return "no data"
        
    
    def CloseHEG(self, evt=None):
        self.lastupdatelen = {}
        if hasattr(self, 'heg') and self.heg and not type(self.heg) == bool:
            if USE_HEGINPUT:
                self.heg.close()
            else:
                self.heg.Close()
        self.heg = None
        
    def OpenCrust(self, evt=None):
        #cf = wx.py.crust.CrustFrame()
        cf = wx.Frame(parent=None)
        cr = wx.py.crust.Crust(cf)
        
        self.crustframes.append((cf, cr))
        cf.Show()
    
    def OpenFilling(self, evt=None):
        if not hasattr(self, 'fillings'):
            self.fillings = []
        self.fillings.append(wx.py.filling.FillingFrame())
        self.fillings[-1].Show()
        
    def InitWaterfall(self, evt=None, comport=None):
        if self.waterfall:
            self.CloseWaterfall()
        if not comport:
            if '--wport' in sys.argv:
                try:
                    comport = int(sys.argv[sys.argv.index('--wport')+1])
                except:
                    traceback.print_exc()
        if comport:
            self.waterfall = waterfall.WaterfallController(comport)
                    
    def CloseWaterfall(self, evt=None):
        if self.waterfall: self.waterfall.Close()
    def PortSearch(self, evt=None):
        pass    
    
    def OnExit(self, evt=None):
        if hasattr(self, 'review'):
            try:
                self.review.Quit()
            except:
                pass
        self.updatetimer.Stop()
        self.SaveSettings()
        self.CloseHEG()
        self.CloseWaterfall()
        time.sleep(0.1)
        if wx.version() < '4':
            wx.App.Exit(self)
        elif evt:
            self.frame.Destroy()
            evt.Skip()
        return 0

    def LoadSettings(self):
        path = u'config.db'
        sp = wx.StandardPaths.Get()
        datadir = sp.GetUserDataDir()
        if DEBUG: 
            print type(datadir)
            print datadir.encode('UTF-8')
            print os.path.exists(datadir)

        if not os.path.exists(datadir):
            os.mkdir(datadir)
            
        # bsddb has trouble opening paths using non-ASCII characters
        # so we chdir to the path to allow the use of a relative path 
        # (in case the directory has a non-ASCII character in it)
        # and then fallback on 
        
        try:
            path = path.encode('ascii')
        except:
            path = path.encode('utf-8').encode('base64')
        
        oldpath = os.getcwdu()    
        #path = opj(datadir, path)
        os.chdir(datadir)
        sh = shelve.open(path)
        os.chdir(oldpath)
        return sh

        #return shelve.open(path)
        
    def SaveSettings(self):
        self.cfg.sync()
        self.cfg.close()
        self.userprofile.sync()
        self.userprofile.close()
    
    def OnUpdate(self, evt=None):
        if self.paused: return
        if not hasattr(self, 'panel'): return
        reads = 0
        while USE_HEGINPUT and self.heg and self.heg.dataWaiting() and reads < 100: # reads < 100 is a timeout for possible bug at EOF
            self.heg.readheg(1)
            reads += 1
        if not USE_HEGINPUT: reads = 1
        try:
            #if len(self.heg.hegdata) < 20:
            #    self.panel.OnResize()
            if hasattr(self.panel, 'mc') and self.panel.mc.GetBestSize() and self.panel.mc.GetBestSize()[0] and not self.panel.loaded:
                #if DEBUG: print "Running OnMediaLoaded"
                self.panel.OnMediaLoaded()
                self.panel.mc.SetSize(self.panel.mc.GetBestSize())
            if (not hasattr(self.panel, 'loaded') or self.panel.loaded) and \
                    (reads > 0 or time.time() - self.lastupdate > 1) and \
                    hasattr(self.heg, 'hegdata') and hasattr(self.heg.hegdata, '__len__'):
                self.lastupdate = time.time()
                self.panel.UpdateData(self.heg.hegdata, self.heg.times)
                if sys.platform == 'win32' and not USE_MPLAYER:
                    movieinterval = 2
                else:
                    movieinterval = 1
                intervals = [(self.panel.UpdateMovie, movieinterval),
                             (self.panel.UpdateText, 4),
                             (self.panel.UpdatePlot, 2+len(self.heg.hegdata)/512)]
                if self.waterfall:
                    intervals += [(self.UpdateWaterfall, 2)]
                for fn, i in intervals:
                    if not i in self.lastupdatelen:
                        self.lastupdatelen[i] = 0
                    if len(self.heg.hegdata) >= self.lastupdatelen[i]+i or (not USE_HEGINPUT and i==1):
                    #if len(self.heg.hegdata) % i - reads < 0:
                        fn(self.heg.hegdata, self.heg.times)
                for fn, i in intervals:
                    if len(self.heg.hegdata) >= self.lastupdatelen[i]+i:
                        self.lastupdatelen[i] = len(self.heg.hegdata)
        except AttributeError:
            if DEBUG: traceback.print_exc()
            pass
        except:# wx._core.PyDeadObjectError: 
            # occurs on some OSes when shutting down
            # print "Dead object encountered when evaluating self.panel.Update(self.heg.hegdata).  Stopping update timer."
            self.updatetimer.Stop()


    def UpdateWaterfall(self, data, times):
        if not self.waterfall:
            return
        HEGlastsecond = numpy.mean(data[-10:])
        HEGlastten    = numpy.mean(data[-100:])
        HEGlastminute = numpy.mean(data[-900:])
        HEGcurrent    = numpy.mean(data[-3:])
        dy  = int(min(200., max(-200., 30.*(HEGcurrent - HEGlastten) + .5)))
        hue = int(min(13.,  max(-10.,    4.*(HEGcurrent - HEGlastsecond) + .5)))
        rgb = [11+hue, 1, 11-hue]
        self.waterfall.SetDY(dy)
        #self.waterfall.SetRGB(rgb)
        
    def OnPlay(self, val=None, evt=None):
        self.InitHEG()
    def OnPause(self, val=None, evt=None):
        if val == None:
            val = not self.paused
        if val:
            self.pause_start = time.time()
        else:
            self.paused_time = time.time() - self.pause_start + self.paused_time
        self.paused = val
        if self.heg:
            if USE_HEGINPUT:
                self.heg.pause(self.paused)
            else:
                self.heg.Pause(self.paused)
    def OnStop(self, val=None, evt=None):
        self.CloseHEG()
    def OnRecord(self, val=None, evt=None):
        return self.SaveSession()

class PortChooseDialog(wx.SingleChoiceDialog):
    def __init__(self, default_port=None, port_options=[], *args, **kwargs):
        if not 'style' in kwargs: kwargs['style'] = 0
        kwargs['style'] = wx.OK|wx.CANCEL|wx.CHOICEDLG_STYLE
        #if not 'title' in kwargs: kwargs['title'] = "COM port selection"
        if not 'message' in kwargs: kwargs['message'] = "Please choose a serial port from the list."
        if not 'caption' in kwargs: kwargs['caption'] = "Port Selection"
        kwargs['choices'] = ["None"] + port_options
        wx.SingleChoiceDialog.__init__(self, *args, **kwargs)

        


if __name__ == '__main__':
    app = MyApp(redirect=('--redirect' in sys.argv))
    app.MainLoop()
