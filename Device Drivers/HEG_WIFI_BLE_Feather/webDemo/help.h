const char help_page[] PROGMEM= R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <style>
            body { background-color: #383838; font-family: Console, Lucida, monospace; font-weight: bold; }
            a:link{
                color:lightblue
            }
            a:visited{
                color:lightblue
            }
            hr{
                border: 2px solid ghostwhite
            }
            body {
                color: white;
            }
            hr.hrhead{
                border: 6px solid ghostwhite
            }
            hr.hr1 {
                border: 4px solid peachpuff
            }
            hr.hr2 {
                border: 2px solid lightgoldenrodyellow
            }
            hr.hr3 {
                border: 2px solid lightgreen           }
            hr.hr4 {
                border: 2px solid gray
            }
        </style>
    </head>
    <body>
        <div id="header">
        <h1> Help page </h1>
        </div>
        <hr class="hrhead">
        <h3> Contents </h3>
        <a href="#s0"> Section 0: Quick Start Guide</a><br>
        <a href="#s1"> Section 1: Introduction</a><br>
        <a href="#s2"> Section 2: Web Demo guide</a><br>
        <a href="#s3"> Section 3: Technical Guide</a><br>
        <br>
        <div id="s0">
        <hr class="hr1">
        <h2> Section 0: Quick Start Guide </h2>
        <hr class="hr1">
        <h3> 0a. Find the HEG WiFi! </h3>
        <p>
        First thing's first: <br>
        <br>
        1. Power up your HEG by plugging it into your computer or plugging a battery in.<br><br>
        2. Open up your WiFi signal list and look for "My_HEG". Connect with the password "12345678".<br><br>
        3. Depending on your OS, open up Chrome or Firefox (recommended) and enter "http://192.168.4.1" or "http://esp32.local" (this works on Apple and Bonjour-enabled devices)<br><br>
        4. Check out the HTML5 demo and see if the HEG LEDs come on when you hit start!<br><br>
        <hr class="hr4">
        <h4>WiFi Settings: </h4>
        From the main page, go to "Connection Settings" or add /connect to the page address. Here you may also connect your HEG through your local WiFi and access it from your main network, enabling online features.<br><br>
        The easiest way to get online is to enter your local WiFi name and password, which can be seen
        via scan results at the bottom of the page. Just type in these to pieces of information and hit connect. <br><br>
        You can now connect to the ESP32 from your main router either via http://esp32.local or an automatically assigned IP address you can find in your router's interface. 
        <br>
        <hr class="hr4">
        You can also assign a "Static IP" if you know the gateway IP and subnet mask, and know an unused local IP address you can assign to the ESP32. <br><br>
        We also have a "Suggest IP" mode which, when you type in the SSID and password, will connect then disconnect to that router and then suggest you an IP. This is done from the
        default ESP32 access point from the /connect page. when you reconnect to the access point. 
        <br><br>
        There is some jank depending on the type of router you connect to,
        so try a few times if it does not work at first. You cannot connect to a 5GHz router on the ESP32.
        </p><br>
        <hr>
        <h3> 0b. Your first session </h3>
        <p>
        Make sure the HEG sensor is firmly flush against your forehead so you can feel the LEDs and photosensor
        pressing against you but without hurting.
        <br><br>
        The easiest way to test your HEG is through our web API. Just go to the HTML5 Web demo from the main page or /listen and hit the "Start HEG" button!
        <ul><li>Chrome or Firefox work the most consistently. </li>
        <li> We are working on full cross-browser compatibility.</li>
        </ul>
        However, you may soon find that after a few seconds, no data comes through, or you get weird 
        negative values or giant oscillations on the graph. This is where you remember you're using a 
        developer product that isn't always consistent if you don't know what you're supposed to do.
        <br><br>
        <hr>
        <h3> 0c. Debugging the signal </h3>
        Check these conditions when you are not receiving a stable, positive ratio and cannot control the games:<br>
        <ul>
        <li> Can you see the Red LED flashing when you hit start?</li>
        <li> Open up the debug console on your web browser and verify if you get "HEGduino Events Connected" on the HTML5 Web Demo page</li>
        <li>Check your ambient lighting conditions. It is best but not required to be in a dim or indirectly lit environment.</li>
            <ul><li>If the light sensor is not firmly against your forehead and exposed directly to the LEDs or sunlight, it will be saturated and output 0's in the Red and IR output.</li>
            <li> Even a cloudy day can create too much ambient light for the LEDs to be picked up, and you will get a negative or zero ratio, or wild oscillations.</li>
            </ul>
        <li> If you use the command 'D' via USB Serial while the sensor is off it will toggle Debug mode and allow you to see the raw values per LED flash.</li>
        <li> If you are receiving 32767 your ADC is being saturated. There are several reasons this may be the case.</li>
            <ul><li> If there is too much ambient light or the LEDs are directly exposed to the photodiode.</li>
            <li> Is your forehead sweaty? This can cause moisture to build up and short the sensor temporarily until it dries again.</li>
                <ul><li> We recommend taping a screen protector down over the sensor to buffer moisture. </li></ul>
            <li> If there is a short or the photodiode is fried</li></ul>
        <li> If you are receiving -1, this means the ADC is not set up or connected correctly.</li>
        <li> If you are receiving repeating low values like 64, 84, etc, that don't produce a stable ratio that means the photodiode output is not being read and may be damaged or not soldered correctly.</li>
        <li> Check that no solder points or leads are contacting your skin as this will ground out those voltages. Electrical tape is an easy way to cover this up.	</li>
        </ul>
        If you cannot get any data through, use a serial monitor (like Arduino or this chrome app: https://chrome.google.com/webstore/detail/arduino-chrome-serial-mon/opgcocnebgmkhcafcclmgfldjhlnacjd?hl=en) <br>
        Follow the instructions in the readme in our github repository (https://github.com/moothyknight/HEG_ESP32) <br><br>
    </p>
        </div>
        <hr>
        <div id="sh">
        <h3> 0d. Safety Precautions and Disclaimers </h3>
        <hr class="hr3">
        <h3>Safety Guide - Know Your Limits! </h3>
        <hr class="hr3">
        <p>
        We've been recommended by clinicians and long-time biofeedback practitioners several
        key conditions to follow when you begin your training to ensure safe and effective results.
        Like any form of exercise, you can strain yourself too much, leading to unpleasant effects 
        or fatigue. The HEG can be a powerful learning and interfacing tool when used correctly.
        <ul>
        <li> Your first sessions should be no more than 5 to 10 minutes between breaks. </li>
        <li> First just try to get a feel for breathing slow, relaxing, focusing, and noticing how your score increases or decreases based on your moment to moment thoughts and movements.</li>
        <li> Limit your first few days of using the HEG to 30 minutes of practice.</li>
        <li> You may feel strain or fatigue after doing the exercises for 10 minutes or more, if you do stop and rest.</li>
        <li> If you "over train" you may get a small headache in the area you trained. This should subside after a few hours or after a night's rest.</li>
            <ul><li> In the unlikely event the headache does not subside the next day, you can "train down" by trying to lower your score while sensing that area, or call your doctor as it may be something else.</li></ul>
        </li>
        </ul>
        </p>
        <hr class="hr3">
        <h3>Disclaimers</h3>
        <hr class="hr3"> <br>
        FDA Disclaimer:<br><br>
        These statements have not been evaluated by the Food and Drug Administration. These statements are for informational purposes only and are not 
        intended as a substitute for medical counseling. This information is not intended to diagnose, treat, cure, or prevent any disease. The author 
        and publisher shall have neither liability nor responsibility to any person or entity with respect to any loss, damage, or injury caused 
        directly or indirectly by the information contained herein.<br><br>
        <hr class="hr4"><br>
        Our Disclaimers:<br><br>
        This is an experimental product by enthusiasts for enthusiasts, we are not health professionals and assume no risks in how you use this, 
        We assert that we have worked to the best of our ability to ensure the safety and enjoyment of our users, whether they are tech savvy or not. 
        <br><br>
        If you have heart or blood pressure problems or are at risk of stroke or other vascular issues, aerobic exercise can help immensely in any form, 
        but be cautious when training your brain blood flow as it is a very sensitive and intricate organ. We do not know all of the potential risks 
        associated when using a device like the HEG if you have serious health conditions.
        <br><br>
        If you are experiencing an episode of a mental or brain disorder, HEG or aerobic exercise in general may help as a form of intervention.
        However, there are complex risk factors associated and we again cannot make any calls as to the best procedure. Please follow your doctor's 
        recommendations and don't put yourself at unnecessary risk.
        <br><br>
        <hr class="hr4"><br>
        Support:<br><br>
        Please inform us of any positive or adverse reactions you have when using the HEG so we can document it. 
        For technical support please create an issue at https://github.com/moothyknight/HEG_ESP32 so we can publicly resolve it to guide future users.<br>
        You may also contact us for questions or technical support at: brewster.joshua1@gmail.com
        <br><br>
        </div>
        <hr class="hr1">
        <div id="s1">
        <h2> Section 1: Introduction </h2>
        <hr class="hr1"> <br>
        <h3> Welcome to HEG Biofeedback with the Internet of Things! </h3>
        <hr>
        <p>
        This hardware and software package is designed to have a minimum barrier 
        for entry for users wanting to use or develop this unique form of brain training. 
        Hemoencephalography (HEG) simply means using blood flow signals for feedback
        data from the body while doing certain actions or experiencing different mental
        states.
        <br>
        <hr class="hr4">
        Hemo - Meaning blood.<br>
        Encephalography - Imaging brain structure through a particular medium.<br>
        <hr class="hr4">
        This device you are using now belongs to particular classification called fNIRS
        or "functional Near-Infrared Spectroscopy" as we are using Red and InfraRed LEDs
        reflected into a light sensor to measure an important component of brain activity.
        It is a reflectance-based pulse oximeter in closest comparison, and we can gather
        the same data, we just use it in a "functional" way to measure overall O2 saturation
        of a particular region of your brain.
        <br><br>
        The HEG sensor is able to capture signals from about an inch deep in your skull
        wherever it has been placed. This is done mainly on our forehead and the pre-frontal
        cortex as this is the most accessible area, but smaller sensors will allow for 
        measuring other parts of the scalp. 
        <br><br>
        The primary usage of these sensors in our case is for direct aerobic exercise for 
        your brain. Normally you energize your brain through normal diet and exercise -
        which add fuel and flexibility to your neurons. Aerobic exercise encourages good
        blood flow and stronger capillary development. It not only stretches your back out
        and loosens your limbs up - but your brain too! This causes energy to be more
        available to your system in general and for it to get more used to stress and strain.
        <br><br>
        Our brain cells require a lot of energy to be immediately available to fire. Neurons receive
        their energy on-demand, chiefly through astrocytes connected to innumerable tiny capillary tubes
        that wrap around your cells in order to sustain them. Stress, poor diet, dehydration,
        or any number of things that can reduce your well-being have some of their strongest
        impacts on brain health due to the incredibly sensitive and intricate ecosystem they form
        in your noggin. 
        <br><br>
        We can see correlations with poor health and poor state of mind directly with HEG by 
        massive reductions or even over-activity in normal blood flow levels, as well as improving 
        it through HEG-assisted exercise. This has overall benefits to your body and mind - which 
        are not really separate. The main benefits are you will feel more aware, quicker on your 
        feet, and able to make better choices when your brain is in shape. The directness of this 
        procedure seems to extend the benefits of normal exercise to your brain in very profound 
        ways, and may shed some light on important ideas about the nature of living consciousness and 
        memory.
        <br><br>
        Before you embark, however, it is important to understand the risks and the experimental 
        nature of this work. While general exposure to the HEG is safe, you can over-train and give 
        yourself headaches or fatigue - kind of like normal exhaustion after a hard exercise. So 
        please read our safety guide and disclaimers in the previous section.
        <br><br>
        </p>
        </div>
        <hr class="hr1">
        <div id="s2">
        <h2> Section 2: Web Demo guide </h2>
        <hr class="hr1"> <br>
        <p>
        Note:<br>
        This web demo is a heavy work in progress. We are working on bringing a much fuller
        suite of activities for our web demo, while what's currently in place is functional
        enough for training. 
        <br>
        <hr class="hr4">
        <h3>How it works: </h3>
        You will see a graph and a circle when you open the web demo. Turn your sensor on via the visible button. After a couple seconds of good data, the graph charts a score
        based off the Simple Moving Average of the ratio. 
        <br><br>
        The score is the cumulative difference between
        1 second and 2 seconds of averaged data, representing a simple "effort" score. The score doesn't actually
        mean anything other than to exaggerate the changes in ratio data being reported by comparing shorter and longer
        term results. This then decides how much the circle increases or decreases in radius per frame. 
        The other data you will see are not used in the current rudimentary feedback setup. <br><br>
        <hr class="hr4">
        <h3>Feedback Options: </h3>
        There are 4 feedback methods currently, though each needs polish. The Circle and Audio
        exercises are the most recommendable ones for now while we continue tuning them. <br><br>
        If you connect the device online, you will find an exciting ThreeJS option. This is a work-in-progress visual where you control the rotation of a planet and make the sun rise or set. ThreeJS enables much more, which we will be working on showing off.<br><br>
        Audio accepts MP3s, Video accepts MP4s. <br><br>
        There is a default video that plays when connected online, as a basic streaming example. <br><br>
        <hr class="hr4">
        <h3>Graph Options:</h3>
        XOffset: Lets you roll back data.<br><br>
        XScale: Changes the number of visible data points.<br><br>
        YScale: Changes the height of the y-axis to make larger or smaller values more visible.<br><br>
        <hr class="hr4">
        <h3>Data Options: </h3>
        The Data menu contains ways to send commands to the device and control the scoring sensitivity.<br>
        <h4>Annotating:</h4>
        Click "Get Time" at any point in your exercise to get the current time stamp. You can then type in a quick (or extensive) note and click "Annotate" for it to show up in the saved CSV.
        <h4>Saving and Replaying Data:</h4>
        Click on the Data tab and look for "Save CSV". The window next to it lets you type in a name or will
        use a default name otherwise. This will create a simple comma-separated-value spreadsheet of your raw session data.
        Google Sheets is an excellent tool for quickly visualizing your data in more detail.
        <br>
        When you click "Replay CSV" a window will open asking for a CSV file in the same format. This will begin streaming the 
        data as if it was playing a session. We are still working on session exploration features as well as timestamped annotating. 
        We will soon also have backend support so data will be able to be saved online to a secure server.
        <h4>Host:</h4>
        This is an option that lets you define a custom EventSource host. This is for if you are not connected directly to the device and want to access it remotely or from a local development client for faster iteration.
        <h4>Output:</h4>
        This shows the raw data coming from the device, this data all is collected in the CSV along with your annotations.
        <br><br>
        
        
        <h4></h4>
        </p>
        </div>
        <hr class="hr1">
        <div id="s3"> 
        <h2> Section 3: Technical Guide </h2>
        <hr class="hr1"><br>
        <h3>Commands</h3>
        't' - Turns sensor on, you'll see a data stream if the sensor is isolated, as in contacting your skin and not exposed to ambient light. <br>
        'f' - Sensor off. <br>
        'W' - Reset WiFi to default access point mode (wipes saved credentials). <br>
        'B' - Toggle Bluetooth Serial mode, resets devices to be in BTSerial (classic) mode. Access like standard USB serial after pairing.
        'b' - Toggle BLE mode, resets the device to start in BLE mode, so you have to enter 'b' again through the serial or BLE connection to switch back to WiFi.<br>
        'R' - hard resets the ESP32.<br>
        's' - soft resets the sensor data.<br>
        'u' - toggles USB data stream. <br>
        'p' - really basic pIR setting. Just turns the LEDs off as the photodiode picks up radiant heat from your body.<br>
        '0','1','2','3' - Changes ADC channel the device reads, in the case of multiple light sensors.<br>
        '5' - Read differential between A0 and A1 on ADS1115 to reduce noise (e.g. connect A1 to signal ground).<br>
        '6' - Read differential between A2 and A3 on ADS1115. <br>
        'D' - toggles ADC debugging (USB Serial output only). <br>
        'L' - toggles LED ambient light cancellation. <br>
        <br>
        <hr class="hr4">
        <h3>Data Output Information</h3>
        <p>
        Output String:<br><br>
        "Current Microseconds | Red LED Sample Average | IR LED Sample Average | Red/IR Ratio Average | Ambient Sample | Velocity (ratio change/ms)| Acceleration (ratio change/ms^2) \r\n" <br><br>
        WiFi: Subscribe to a Server Sent Event stream (EventSource) at the IP of the device. Its update period is set at 50ms. The data stream comes through on "heg" events.<br><br>
        USB Serial: Via a serial monitor, set baud rate to 115200 on the correct COM port to read the data, debug, and send commands.<br><br>
        Bluetooth: Requires scanning for BLE device and subscribing to the correct UART characteristic, then sending the 't' command to activate the sensor stream.<br><br>
        </p>
        <hr class="hr4">
        <h3>API page</h3>
        <p>
        This is an experimental page that lets you call all of the user interface yourself and define what default features you want enabled. The initial command allows you to define arbitrary
        EventSource location for if your HEG is not in its default setup or if you want to access it remotely. 
        <br><br>
        It's possible to subscribe to multiple sessions remotely this way. We hope this 
        gives developers an easier ability to work with our applications and help unify open source development. We are developing a web-based way to access
        the device too, to open up the vast software resources online connectivity provides.
        </p>
        <hr class="hr4">
        <h3>External Links:</h3>
        <ul>
            <li><a href="https://github.com/moothyknight/HEG_ESP32/blob/master/HEG_WIFI_BLE/README.txt">ReadMe & Changelog (important info for building drivers yourself)</a></li>
            <li><a href="https://github.com/moothyknight/HEG_ESP32/blob/master/HEG%20Whitepaper.pdf">Useful References (at bottom of PDF)</a></li>
            <li><a href="https://github.com/espressif/arduino-esp32">Arduino ESP32</a></li>
            <li><a href="https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf">ESP32 Technical Manual</a></li>
            <li><a href="https://www.arduino.cc/en/Guide/HomePage">Arduino Guide</a></li>
        </ul>
        </div>
        <hr class="hrhead">
    </body>
    </html>
)=====";
