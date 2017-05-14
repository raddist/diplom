function [psnr bpp] = plotPSNR

imageName  = 'lena.bmp';
I = imread(imageName);
quant = 10; %10

N = 8; %8
bpp = ones(N,5);
psnr = ones(N,1);


for i = 1:N
    % call func to encode first file
    command = strcat('waveCompression.exe 0',32,imageName,32,'test4.bmp',32,num2str(quant) );
    system(command);
    
    %mapped
    fid = fopen('map_encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
 
    cnt=1;              % инициализация счетчика 
    while ~feof(fid)    % цикл, пока не достигнут конец файла 
        [V,N] = fread(fid, 1, 'int8');  %считывание одного 

        if N > 0        % если элемент был прочитан успешно, то 
        cnt=cnt+1;  % увеличиваем счетчик на 1 
        end
    end
    fclose(fid);
    
    J = imread('test4.bmp');
    bpp(i,1) = cnt / size(I,1) / size(I,2) * 8;
    psnr(i) = myPSNR(I,J);
    
    % one-model
    fid = fopen('encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
    
    cnt=1;              % инициализация счетчика 
    while ~feof(fid)    % цикл, пока не достигнут конец файла 
        [V,N] = fread(fid, 1, 'int8');  %считывание одного 

        if N > 0        % если элемент был прочитан успешно, то 
        cnt=cnt+1;  % увеличиваем счетчик на 1 
        end
    end
    fclose(fid);
    bpp(i,2) = cnt / size(I,1) / size(I,2) * 8;
     
    % context
    fid = fopen('con_encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
    
    cnt=1;              % инициализация счетчика 
    while ~feof(fid)    % цикл, пока не достигнут конец файла 
        [V,N] = fread(fid, 1, 'int8');  %считывание одного 

        if N > 0        % если элемент был прочитан успешно, то 
        cnt=cnt+1;  % увеличиваем счетчик на 1 
        end
    end
    fclose(fid); 
    bpp(i,3) = cnt / size(I,1) / size(I,2) * 8;
    
    % sign context
    fid = fopen('s_encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
    
    cnt=1;              % инициализация счетчика 
    while ~feof(fid)    % цикл, пока не достигнут конец файла 
        [V,N] = fread(fid, 1, 'int8');  %считывание одного 

        if N > 0        % если элемент был прочитан успешно, то 
        cnt=cnt+1;  % увеличиваем счетчик на 1 
        end
    end
    fclose(fid); 
    bpp(i,4) = cnt / size(I,1) / size(I,2) * 8;
    
    % context for sign context
    fid = fopen('sc_encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
    
    cnt=1;              % инициализация счетчика 
    while ~feof(fid)    % цикл, пока не достигнут конец файла 
        [V,N] = fread(fid, 1, 'int8');  %считывание одного 

        if N > 0        % если элемент был прочитан успешно, то 
        cnt=cnt+1;  % увеличиваем счетчик на 1 
        end
    end
    fclose(fid); 
    bpp(i,5) = cnt / size(I,1) / size(I,2) * 8;
    
    quant = quant*3/2;

    figure();
    imshow(J);
end 

plot(bpp(:,1), psnr,'b',...
    bpp(:,2), psnr,'r',...
    bpp(:,3), psnr,'g',...
    bpp(:,4), psnr,'k',...
    bpp(:,5), psnr,'b');
grid on;
xlabel('bpp');
ylabel('PSNR');
title('image compresion');
legend('256 models','1 model','context encoding','sign context','context for sign',...
    'Location','NorthWest');
end
