const char help_page[] PROGMEM= R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <link rel="stylesheet" type="text/css" href="webDemoCSS.css">
        <style>
            body {
                color: white;
            }
        </style>
    </head>
    <body>
        <div id="header">
        <h1> Help page </h1>
        </div>
        <hr>
        <h3> Contents </h3>
        <a href="#s0"> Section 0: Quick Start Guide</a><br>
        <a href="#s1"> Section 1: Introduction</a><br>
        <a href="#s2"> Section 2: Features!</a><br>
        <a href="#s3"> Section 3: Technical Manual</a><br>
        <div id="s0">
        <h2> Section 0: Quick Start Guide </h2>
        <hr>
        <h3> 0a. Find the HEG WiFi! </h3>
        <p>
        First thing's first. <br>
        <br>
        1. Power up your HEG by plugging it into your computer or plugging a battery in.<br>
        2. Open up your WiFi signal list and look for "My_HEG". Connect with the password "12345678".<br>
        3. Depending on your OS, open up Chrome or Firefox (recommended) and enter "http://esp32.local" or "http://192.168.4.1" if that doesn't work. This will bring you to a web interface.<br>
        4. Check out the Canvas demo and see if the HEG LEDs come on when you hit start!<br>
        <br><br>
        You may also connect your HEG through your local WiFi and access it from your main network, enabling online features.<br>
        <br>
        From the main page, go to "Connection Settings" or /connect. The easiest way to get online is to enter your local WiFi name and password, which can be seen
        via scan results at the bottom of the page. Just type in these to pieces of information and hit connect. You can now connect to the ESP32 from your main router
        either via http://esp32.local or an automatically assigned IP address you can find in your router's interface. 
        <br><br>
        You can also assign a "Static IP" if you know the gateway IP and subnet mask, and know an unused local IP address you can assign to the ESP32. We also have a 
        "Suggest IP" mode which, when you type in the SSID and password, will connect then disconnect to that router and then suggest you an IP. This is done from the
        default ESP32 access point from the /connect page. when you reconnect to the access point. There is some jank depending on the type of router you connect to,
        so try a few times if it does not work at first. You cannot connect to a 5GHz router on the ESP32.<br>
        </p>
        <hr>
        <h4> 0b. Your first session </h4>
        <p>
        Make sure the HEG sensor is firmly flush against your forehead so you can feel the LEDs and photosensor
        pressing against you but without hurting. You must be in dim indirect lighting for best results.
        <br><br>
        The easiest way to test your HEG is through our web API. Just go to the<br>
        Web demo from the main page or /webDemo and hit the "Start HEG" button!<br>
        <ul><li>Chrome or Firefox work the most consistently. </li>
        <li> We are working on full cross-browser compatibility.</li>
        </ul>
        However, you may soon find that after a few seconds, no data comes through, or you get weird <br>
        negative values or giant oscillations on the graph. This is where you remember you're using a <br>
        developer product that isn't always consistent if you don't know what you're supposed to do.<br>
        <br><br>
        <hr>
        <h4> 0c. Debugging the signal </h4>
        Check these conditions when you are not receiving a stable, positive ratio and cannot control the games:<br>
        <ul>
        <li> Can you see the Red LED flashing when you hit start?</li>
        <li> Open up the debug console on your driver and verify if you get "HEGduino Events Connected" on the Web Demo page</li>
        <li>Check your ambient lighting conditions. It is best but not required to be in a dim or indirectly lit environment.</li>
            <ul><li>If the light sensor is not firmly against your forehead and exposed directly to the LEDs or sunlight, it will be saturated and will not output data.</li>
            <li> Even a cloudy day can create too much ambient light for the LEDs to be picked up, and you will get a negative or zero ratio.</li>
            </ul>
        <li> If you use the command 'D' while the sensor is off it will toggle Debug mode and allow you to see the raw values per LED flash.</li>
        <li> If you are receiving 32767 your ADC is being saturated. There are several reasons this may be the case.</li>
            <ul><li> If there is too much ambient light or the LEDs are directly exposed to the photodiode.</li>
            <li> Is your forehead sweaty? This can cause moisture to build up and short the sensor temporarily until it dries again.</li>
                <ul><li> We recommend taping a screen protector down over the sensor to buffer moisture. </li></ul>
            <li> If there is a short or the photodiode is fried</li></ul>
        <li> If you are receiving -1, this means the ADC is not set up or connected correctly.</li>
        <li> If you are receiving repeating low values like 64, 84, etc, that means the photodiode output is not being read and may be damaged or not soldered correctly.</li>
        <li> Check that no solder points or leads are contacting your skin as this will ground out those voltages. Electrical tape is an easy way to cover this up.	</li>
        </ul>
        If you cannot get any data through, use a serial monitor (like Arduino or this chrome app: https://chrome.google.com/webstore/detail/ohncdkkhephpakbbecnkclhjkmbjnmlo) <br>
        Follow the instructions in the readme in our github repository (https://github.com/moothyknight/HEG_ESP32) <br>
    </p>
        </div>
        <hr>
        <div id="sh">
        <h2> Safety Precautions and Disclaimers </h2>
        <h3>Safety Guide - Know Your Limits! </h3>
        <p>
        We've been recommended by clinicians and long-time biofeedback practitioners several
        key conditions to follow when you begin your training to ensure safe and effective results.
        Like any form of exercise, you can strain yourself too much, leading to unpleasant effects 
        or fatigue. The HEG can be a powerful learning and interfacing tool when used correctly.
        <br><br>
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
        <hr>
        <h3>Disclaimers</h3>
        FDA Disclaimer:<br>
        These statements have not been evaluated by the Food and Drug Administration. These statements are for informational purposes only and are not 
        intended as a substitute for medical counseling. This information is not intended to diagnose, treat, cure, or prevent any disease. The author 
        and publisher shall have neither liability nor responsibility to any person or entity with respect to any loss, damage, or injury caused 
        directly or indirectly by the information contained herein.
        <br><br>
        Our Disclaimers:<br>
        This is an experimental product by enthusiasts for enthusiasts, we are not health professionals and assume no risks in how you use this, 
        We assert that we have worked to the best of our ability to ensure the safety and enjoyment of our users, whether they are tech savvy or not. 
        <br><br>
        If you have heart or blood pressure problems or are at risk of stroke or other vascular issues, aerobic exercise can help immensely in any form, 
        but be cautious when training your brain blood flow as it is a very sensitive and intricate organ. We do not know all of the potential risks 
        associated when using a device like the HEG if you have serious health conditions.
        <br><br>
        If you are experiencing an episode of a mental or brain disorder, HEG or aerobic exercise in general may help as a form of intervention, 
        but there are complex risk factors associated and we again cannot make any calls as to the best procedure. Please follow your doctor's 
        recommendations and don't put yourself at unnecessary risk.
        <br><br>
        Please inform us of any positive or adverse reactions you have when using the HEG so we can document it. <br>
        For technical support please create an issue at https://github.com/moothyknight/HEG_ESP32 so we can publicly resolve it to guide future users.<br>
        You may also contact us for questions or technical support at: brewster.joshua1@gmail.com<br>
        <br><br>
        </div>
        <hr>
        <div id="s1">
        <h2> Section 1: Introduction </h2>
        <hr>
        <h3> Welcome to HEG Biofeedback with the Internet of Things! </h3>
        <p>
        This hardware and software package is designed to have a minimum barrier 
        for entry for users wanting to use or develop this unique form of brain training. 
        Hemoencephalography (HEG) simply means using blood flow signals for feedback
        data from the body while doing certain actions or experiencing different mental
        states.
        <br><br>
        Hemo - Meaning blood.<br>
        Encephalography - Imaging brain structure through a particular medium.<br>
        <br><br>
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
        Our brain cells require energy to be immediately available to fire. Neurons receive
        their energy on-demand through astrocytes connected to innumerable tiny capillary tubes
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
        ways, and may shed light some important ideas about the nature of living consciousness and 
        memory.
        <br><br>
        Before you embark, however, it is important to understand the risks and the experimental 
        nature of this work. While general exposure to the HEG is safe, you can over-train and give 
        yourself headaches or fatigue - kind of like normal exhaustion after a hard exercise. So 
        please learn our recommendations and some of the known side-effects when you are first 
        starting with HEG brain exercises.
        <br><br>
        </p>
        </div>
        <hr>
        <div id="s2">
        <h2> Section 2: Web Demo guide </h2>
        <hr>
        <h3> Features! </h3>
        <p></p>
        </div>
        <hr>
        <div id="s3"> 
        <h2> Section 3: Technical Guide </h2>
        <hr>
        Commands:<br>
        't' - Turns sensor on, you'll see a data stream if the sensor is isolated, as in contacting your skin and not exposed to ambient light. <br>
        'f' - Sensor off. <br>
        'W' - Reset WiFi to default access point mode (wipes saved credentials). <br>
        'b' - Toggle BLE mode, this resets the device to start in BLE mode, so you have to enter 'b' again through the serial or BLE connection to switch back to WiFi.<br>
        'R' - hard resets the ESP32.<br>
        's' - soft resets the sensor data.<br>
        'u' - toggles USB data stream. <br>
        'p' - really basic pIR setting. Just turns the LEDs off as the photodiode picks up radiant heat from your body.<br>
        '0','1','2','3' - Changes ADC channel the device reads, in the case of multiple light sensors.<br>
        '5' - Read differential between A0 and A1 on ADS1115 to reduce noise (e.g. connect A1 to signal ground).<br>
        '6' - Read differential between A2 and A3 on ADS1115. <br>
        'D' - toggles ADC debugging (USB Serial output only). <br>
        'L' - toggles LED ambient light cancellation. <br>
        <ul>
            <li><a href="https://github.com/moothyknight/HEG_ESP32/blob/master/HEG_WIFI_BLE/README.txt">ReadMe & Changelog (important info for building drivers yourself)</a></li>
            <li><a href="https://github.com/moothyknight/HEG_ESP32/blob/master/HEG%20Whitepaper.pdf">Useful References (at bottom of PDF)</a></li>
            <li><a href="https://github.com/espressif/arduino-esp32">Arduino ESP32</a></li>
            <li><a href="https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf">ESP32 Technical Manual</a></li>
            <li><a href="https://www.arduino.cc/en/Guide/HomePage">Arduino Guide</a></li>
        </ul>
        </div>
    </body>
    </html>
)=====";
