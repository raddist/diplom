function  res = func(lim)



command = strcat('waveCompression.exe 5 goldhill.bmp test4.bmp'...
        ,32,'20 0.1 1.1 3.5',32,num2str(-lim),32,num2str(lim) );
[q w] = system(command);
res = str2num(w);