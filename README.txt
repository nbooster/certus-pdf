This is an attempt to make pdf creation an easy, fast and language agnostic process.

Ideal for server applications (run on multicore systems with NVMe drives) that need to generate pdf reports very frequently.

Supports many commands for any basic (or rich especially with the help of images) 
pdf report with many more to be added and uppon requests or contributions.

In a modern Linux server cloud environment it generates a 10 (or as many threads supported by the underlying system) 
page rich pdf report in 50 to 500 milliseconds (highly depends on the content and images size).

*It also has a command for automatic qr-code image in a pdf page from a string input.


Many thanks to these libs authors:

https://github.com/galkahana/PDF-Writer
https://github.com/RaymiiOrg/cpp-qr-to-png

without them, this project could not be possible,
and https://certusonline.com for sponsoring it.



Creates one thread per input page... so have that in mind (possible to make that optional or dynamic).

See the main.cpp example for usage (very simple).


Compile:

Create a 'build' folder inside libs\PDF-Writer-master
and then inside it run: cmake .. && make
and then go to the root folder and run the compilation command (Linux).
Then run: ./main 

Will to add cmake support fo the whole project...

Compilation command:

g++ -O3 
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/PDFWriter 
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibAesgm 
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/FreeType
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibJpeg 
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibPng 
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibTiff 
-L/home/notis/CertusPDF/libs/PDF-Writer-master/build/Zlib
-I/home/notis/CertusPDF/libs/PDF-Writer-master/FreeType/include 
-I/home/notis/CertusPDF/include 
main.cpp 
include/certusPDF.cpp include/QrToPng.cpp include/base64.cpp include/TinyPngOut.cpp include/QrCode.cpp include/pdfBuffer.cpp include/AnnotationsWriter.cpp include/helpers.cpp 
-o main -std=c++14 
-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wpedantic -march=native 
-lPDFWriter -lLibAesgm -lFreeType -lLibJpeg -lLibPng -lLibTiff -lZlib -pthread

Copy-Paste command:

g++ -O3 -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/PDFWriter -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibAesgm -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/FreeType -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibJpeg -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibPng -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/LibTiff -L/home/notis/CertusPDF/libs/PDF-Writer-master/build/Zlib -I/home/notis/CertusPDF/libs/PDF-Writer-master/FreeType/include -I/home/notis/CertusPDF/include main.cpp include/certusPDF.cpp include/QrToPng.cpp include/base64.cpp include/TinyPngOut.cpp include/QrCode.cpp include/pdfBuffer.cpp include/AnnotationsWriter.cpp include/helpers.cpp -o main -std=c++14 -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wpedantic -ffast-math -march=native -lPDFWriter -lLibAesgm -lFreeType -lLibJpeg -lLibPng -lLibTiff -lZlib -pthread


Input Commands Format:

# Line
L, fromX, fromY, toX, toY, lineWidth, lineColor, lineOpacity

# Image
I, X, Y, scaleX, scaleY, imageOpacity, imageType, base64ImageDataBytes

# QRCodeImage from string
Q, X, Y, size, imageOpacity, string

# Rectangle
R, left, bottom, width, height, reactangleColor, rectangleOpacity, fill

# Circle
C, centerX, centerY, radius, circleColor, circleOpacity, fill

# Polygon
P, polygonColor, polygonOpacity, fill, p1X, p1Y, ...

# Hyperlink
S, lowerLeftX, lowerLeftY, upperRightX, upperRightY, hyperlink

# Simple text line
T, startX, startY, fontSize, textColor, textOpacity, text

# Justified text line
A, startX, startY, endX, fontSize, textColor, textOpacity, text

# Highlighted text line
H, startX, startY, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Highlighted-Justified text line
W, startX, startY, endX, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Simple text line with hyperlink
TS"hyperlink", startX, startY, fontSize, textColor, textOpacity, text

# Justified text line with hyperlink
AS"hyperlink", startX, startY, endX, fontSize, textColor, textOpacity, text

# Highlighted text line with hyperlink
HS"hyperlink", startX, startY, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Highlighted-Justified text line with hyperlink
WS"hyperlink", startX, startY, endX, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Underlined text line
TU, startX, startY, fontSize, textColor, textOpacity, text

# Justified-Underlined text line
AU, startX, startY, endX, fontSize, textColor, textOpacity, text

# Highlighted-Underlined text line
HU, startX, startY, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Highlighted-Justified-Underlined text line
WU, startX, startY, endX, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Underlined text line with hyperlink
TUS"hyperlink", startX, startY, fontSize, textColor, textOpacity, text

# Justified-Underlined text line with hyperlink
AUS"hyperlink", startX, startY, endX, fontSize, textColor, textOpacity, text

# Highlighted-Underlined text line with hyperlink
HUS"hyperlink", startX, startY, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text

# Highlighted-Justified-Underlined text line with hyperlink
WUS"hyperlink", startX, startY, endX, fontSize, textColor, textOpacity, highlightColor, highlihtOpacity, text


Examples:

"H, 5, 790, 25, 0358FF, 1, FFFF00, 1, VEHICLE E-TICKET"
"T, 335, 785, 12, 0358FF, 1, ΔΟΥ ΠΛΟΙΩΝ ΠΕΙΡΑΙΑ, ΑΦΜ: 997595200"
"L, 5, 775, 590,775,2,000000, 0.5",
"I, 5, 675, 0.9, 0.9, 1,JPG, /9j/4AA..."
"R, 5, 790, 215, 20, FF0000, 1, Y"
"R, 5, 790, 215, 20, FF0000, 1, N"
"C, 100, 100.5, 50.5, FF0000, 0.5, Y"
"P, 00FF00, 1, N, 1, 1, 2, 2, 3, 3"
"WUS"google.com", 10, 10, 50, 15, 000000, 0.9, 0358F0, 0.8, googleLink"

The more complex the command the more time it takes (WUS and commands with images take the longest...)
