This project is currently set to connect to a raspberry pi model B on a local network. To begin, get your raspberry pi's IP and run the following console command on your pc:

plink -pw abc craft@10.0.0.41
(replace 10.0.0.41 with your ip)

Then navigate to the RaspPiGPIOTest.cpp file and set PI_HOST to your ip