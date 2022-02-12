# ledmatrix32x8
controlling an led matrix with esp8266 using web interfaces and wifi acces point

## Preview of the Fourier Transform Functionality
This is a snippet of the fourier transform of [Thunder by Netsky](https://open.spotify.com/track/0AQAdESuhbfV2ldTpO2MYF?si=XHX3PpR1S5uot5eVFank0A&context=spotify%3Asearch%3Athunder%2520).
<img src="https://github.com/JF0C/ledmatrix32x8/blob/master/src/fourier%20transform%20.gif"/>

## Setup Software
set up arduino IDE: https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/  
esp8266 board library version 3.0.1 was used  
ArduinoJson version 6.18.3  
FastLED version 3.4.0  
in webinterface.ino line 9 uncomment and change ssid and password to match your wifi acces point  
after flashing and starting the esp read the ip displayed on the serial monitor / led matrix  
go to <ip>/upload and upload all files of this repository ending with .js, .html, .json and .css  
this document helps a lot at understanding and programming the esp8266 https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html    

## Setup Hardware
led matrix 5V 32x8 was used something like: https://www.amazon.de/BTF-LIGHTING-RGB-Legierung-adressierbar-FCB-Vollfarbe-funktioniert/dp/B088K1KDW5/ref=sr_1_5?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=led+matrix&qid=1628182227&sr=8-5  
depending on the led layout of your matrix, adapt the ```drawxy``` function in line 289 in matrix.ino  
depending on the color encoding (sequence of red, green, blue values) change FastLED setup in line 93 in matrix.ino  
pin D5 is used for led control (can be changed in line 2 in matrix.ino)  
pin A0 is used for audio input (can be changed in line 19 in matrix.ino)  
A logic level converter 3.3V to 5V is required (something like: https://www.amazon.de/Gebildet-Pegelwandler-Converter-BiDirektional-Mikrocontroller/dp/B07RY15XMJ/ref=sr_1_3_sspa?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=2Q6B1HCRMKGMJ&dchild=1&keywords=logic+level+converter+3.3v+5v&qid=1628182371&sprefix=logic+level+%2Caps%2C192&sr=8-3-spons&psc=1&spLa=ZW5jcnlwdGVkUXVhbGlmaWVyPUEzOEhXQjQ4OERMMVdLJmVuY3J5cHRlZElkPUEwNjYxNjg1S044RzRaRks5U0VQJmVuY3J5cHRlZEFkSWQ9QTA1NTU2NjkzUVNMQ0VCVkNOMVpPJndpZGdldE5hbWU9c3BfYXRmJmFjdGlvbj1jbGlja1JlZGlyZWN0JmRvTm90TG9nQ2xpY2s9dHJ1ZQ==)  
during start up of the esp do not connect the 5V pin of the logic converter as this will cause an error (a switch can help here)  



                            |~~~~~~~|
                            |       |
                            |       |
                            |       |
                            |       |
                            |       |
 |~.\\\_\~~~~~~~~~~~~~~xx~~~         ~~~~~~~~~~~~~~~~~~~~~/_//;~|
 |  \  o \_         ,XXXXX),                         _..-~ o /  |
 |    ~~\  ~-.     XXXXX`)))),                 _.--~~   .-~~~   |
  ~~~~~~~`\   ~\~~~XXX' _/ ';))     |~~~~~~..-~     _.-~ ~~~~~~~
           `\   ~~--`_\~\, ;;;\)__.---.~~~      _.-~
             ~-.       `:;;/;; \          _..-~~
                ~-._      `''        /-~-~
                    `\              /  /
                      |         ,   | |
                       |  '        /  |
                        \/;          |
                         ;;          |
                         `;   .       |
                         |~~~-----.....|
                        | \             \
                       | /\~~--...__    |
                       (|  `\       __-\|
                       ||    \_   /~    |
                       |)     \~-'      |
                        |      | \      '
                        |      |  \    :
                         \     |  |    |
                          |    )  (    )
                           \  /;  /\  |
                           |    |/   |
                           |    |   |
                            \  .'  ||
                            |  |  | |
                            (  | |  |
                            |   \ \ |
                            || o `.)|
                            |`\\\\) |
                            |       |
                            |       |
                            |       |
