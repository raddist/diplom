function  res = func(lim, q)



command = strcat('waveCompression.exe 5 goldhill.bmp test4.bmp'...
        ,32,num2str(q),32,' 0.1 1.1 3.5',32,num2str(-lim),32,num2str(lim) );
[i w] = system(command);
res = str2num(w);