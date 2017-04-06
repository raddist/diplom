function findOpt



command = strcat('waveCompression.exe 3 lena.bmp test4.bmp'...
        ,32,num2str(0.1),32,num2str(1),32,num2str(5) );
[q w] = system(command);

d = 10