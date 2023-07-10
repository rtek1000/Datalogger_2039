
The Datalogger generates BMP files when it captures the screen with the PrintScreen keyboard command. But GitHub works best with JPG, so the BMP images were converted to JPG using the command line below on Linux:

>mogrify -quality 100 -format jpg *.bmp

To install imagemagick on Ubuntu:
>sudo apt install imagemagick

Ref.: https://superuser.com/questions/26935/convert-a-bunch-of-bmp-files-to-jpeg-on-linux
