#ifndef _WEBPAGE_H
#define _WEBPAGE_H

#define PART_BOUNDARY "123456789000000000000987654321"

#define _STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=" PART_BOUNDARY
#define _STREAM_BOUNDARY "\r\n--" PART_BOUNDARY "\r\n"
#define _STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"

const char INDEX_HTML[] = R"rawliteral(
<html>

<head>
    <title>PixelBit FPV RockBot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial;
            text-align: center;
            margin: 0px auto;
            padding-top: 30px;
        }

        table {
            margin-left: auto;
            margin-right: auto;
        }

        td {
            padding: 8 px;
        }

        .button {
            background-color: #2f4468;
            border: none;
            color: white;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 18px;
            margin: 6px 3px;
            cursor: pointer;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            -khtml-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
        }

        img {
            width: auto;
            max-width: 100%;
            height: auto;
        }
    </style>
</head>

<body>
    <h1>PixelBit FPV RockBot</h1>
    <img src="" id="photo">
    <table>
        <tr>
            <td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('forward');"
                    ontouchstart="toggleCheckbox('forward');" onmouseup="toggleCheckbox('stop');"
                    ontouchend="toggleCheckbox('stop');">Forward</button></td>
        </tr>
        <tr>
            <td align="center"><button class="button" onmousedown="toggleCheckbox('left');"
                    ontouchstart="toggleCheckbox('left');" onmouseup="toggleCheckbox('stop');"
                    ontouchend="toggleCheckbox('stop');">Left</button></td>
            <td align="center"><button class="button" onmousedown="toggleCheckbox('stop');"
                    ontouchstart="toggleCheckbox('stop');">Stop</button></td>
            <td align="center"><button class="button" onmousedown="toggleCheckbox('right');"
                    ontouchstart="toggleCheckbox('right');" onmouseup="toggleCheckbox('stop');"
                    ontouchend="toggleCheckbox('stop');">Right</button></td>
        </tr>
        <tr>
            <td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('backward');"
                    ontouchstart="toggleCheckbox('backward');" onmouseup="toggleCheckbox('stop');"
                    ontouchend="toggleCheckbox('stop');">Backward</button></td>
        </tr>
        <tr>
            <td align="center"><button class="button" onmousedown="toggleCheckbox('pan_left');"
                    ontouchstart="toggleCheckbox('pan_left');" onmouseup="toggleCheckbox('stop');"
                    ontouchend="toggleCheckbox('stop');">Pan Left</button></td>
            <td></td>
            <td align="center"><button class="button" onmousedown="toggleCheckbox('pan_right');"
                    ontouchstart="toggleCheckbox('pan_right');" onmouseup="toggleCheckbox('stop');"
                    ontouchend="toggleCheckbox('stop');">Pan Right</button></td>
        </tr>
        <tr>
            <td align="right" valign="bottom" style="width:40px;height:40px"></td>
            <td valign="bottom" style="width:40px;height:40px">
                <center><input type="range" min="0" max="90" step="1" oninput="sliderChange(this.value)"
                        onchange="sliderChange(this.value)" /></center>
            </td>
            <td valign="bottom" style="width:40px;height:40px"></td>
        </tr>
    </table>
    <script>
        function toggleCheckbox(x) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=" + x, true);
            xhr.send();
        }
        function sliderChange(x) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?slider=" + x, true);
            xhr.send();
        }
        window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
    </script>
</body>

</html>
)rawliteral";

#endif