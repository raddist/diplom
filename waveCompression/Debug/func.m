function  res = func(lim)



command = strcat('waveCompression.exe 3 lena.bmp test4.bmp'...
        ,32,num2str(lim(2)),32,num2str(lim(2)),32,num2str(lim(3)) );
[q w] = system(command);
res = str2num(w);