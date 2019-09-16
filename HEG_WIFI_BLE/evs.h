const char event_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
body {
    background-color: gray
}
scoreDiv {
    color: blue
}
</style>
</head>
<body id="main_body">

    <script>
        var ms = [];
        var red = [];
        var ir = [];
        var ratio = [];
        var smallSavLay = [];
        var largeSavLay = [];
        var adcAvg = [];
        var ratioSlope = [];
        var AI = [];
        var slowFastScore = [100];
        var slowSMAArray = [];

        //appendId is the element Id you want to append this fragment to
        function appendFragment(HTMLtoAppend, appendId) {

            fragment = document.createDocumentFragment();
            var newDiv = document.createElement('div');
            newDiv.innerHTML = HTMLtoAppend;
            newDiv.setAttribute("id", appendId + '_child');

            fragment.appendChild(newDiv);

            document.getElementById(appendId).appendChild(fragment);
        }

        //delete selected fragment. Will delete the most recent fragment if Ids are shared.
        function deleteFragment(parentId,fragmentId) {
            this_fragment = document.getElementById(fragmentId);
            document.getElementById(parentId).removeChild(this_fragment);
        }

        //Separates the appendId from the fragmentId so you can make multiple child threads with different Ids
        function appendFragmentMulti(HTMLtoAppend, appendId, fragmentId) {

            fragment = document.createDocumentFragment();
            var newDiv = document.createElement('div');
            newDiv.innerHTML = HTMLtoAppend;
            newDiv.setAttribute("id", fragmentId + '_child');

            fragment.appendChild(newDiv);

            document.getElementById(appendId).appendChild(fragment);

        }

        function slowFastSMAScore(dataArray) {
            var dy = 0;
            var slowSMA = 0;
            var fastSMA = 0;
            var score = 0;
            if(dataArray.length > 20) {
                for(var i=dataArray.length-20;i<dataArray.length-1;i++) {
                    slowSMA += dataArray[i];
                    if (i > dataArray.length - 6) {
                        fastSMA += dataArray[i];
                    }
                }
                slowSMA = slowSMA / 20;
                slowSMAArray.push(slowSMA);

                fastSMA = fastSMA / 5;
                dy = fastSMA - slowSMAArray[slowSMAArray.length - 1];
                score = slowFastScore[slowFastScore.length - 1] + dy;
                slowFastScore.push(score);
            }
        }

        var containerHTML = '<div id="container"></div>';
        var messageHTML = '<div id="message">Not connected</div>';
        var eventHTML = '<div id="myevent">Waiting...</div>';
        var scoreHTML = '<scoreDiv id="ScoreHTML">Getting score...</scoreDiv>';

        appendFragment(containerHTML,"main_body");
        appendFragment(messageHTML,"container");
        appendFragment(eventHTML,"container");
        appendFragment(scoreHTML,"container");

        if (!!window.EventSource) {
            var source = new EventSource('/events');

            source.addEventListener('open', function(e) {
                console.log("Events Connected");
            }, false);

            source.addEventListener('error', function(e) {
                if (e.target.readyState != EventSource.OPEN) {
                console.log("Events Disconnected");
                }
            }, false);

            source.addEventListener('message', function(e) {
                document.getElementById("message").innerHTML = e.data;
                console.log("message", e.data);
            }, false);

            source.addEventListener('myevent', function(e) {
                document.getElementById("myevent").innerHTML = e.data;
                console.log("myevent", e.data);
                if(e.data.includes("|")) {
                    var dataArray = e.data.split("|");
                    ms.push(parseInt(dataArray[0]));
                    red.push(parseInt(dataArray[1]));
                    ir.push(parseInt(dataArray[2]));
                    ratio.push(parseFloat(dataArray[3]));
                    smallSavLay.push(parseFloat(dataArray[4]));
                    largeSavLay.push(parseFloat(dataArray[5]));
                    adcAvg.push(parseInt(dataArray[6]));
                    ratioSlope.push(parseFloat(dataArray[7]));
                    AI.push(parseFloat(dataArray[8]));

                    //slowFastSMAScore(ratio);
                    document.getElementById("ScoreHTML").innerHTML = ratio[ratio.length-1];//slowFastScore[slowFastScore.length - 1]
                }
            
        }, false);

        //data = e.data;
        //if(typeof(Storage) !=="undefined") 
        //{window.localStorage.setItem('StateChangerPlugin', data);}
        //StateChangerPlugin.message(data);
        }
    </script>

    <form method="post" action="/startHEG" target="dummyframe">
        <button type="submit">Start HEG</button>
    </form>
    <form method="post" action="/stopHEG" target="dummyframe">
        <button type="submit">Stop HEG</button>
    </form>

    <iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe"></iframe>


</body>
</html>
)=====";
