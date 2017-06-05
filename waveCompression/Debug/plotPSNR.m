function [psnr bpp] = plotPSNR

imageName  = 'barbara.bmp';
I = imread(imageName);
quant = 10; %10
quality = 75;

N = 8; %8
bpp = ones(N,7);
psnr = ones(N,3);


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
    psnr(i,1) = myPSNR(I,J);
    
    figure();
    imshow(J);
    
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
    
    % jpeg 2000
    imwrite(I,'goldhill.jp2','jp2','CompressionRatio',quant);
    fid = fopen('goldhill.jp2', 'rb');
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
    J = imread('goldhill.jp2');
    psnr(i,2) = myPSNR(I,J);
    bpp(i,6) = cnt / size(I,1) / size(I,2) * 8;

    % jpeg
    imwrite(I,'goldhill.jpg','jpg','Quality',quality);
    fid = fopen('goldhill.jpg', 'rb');
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
    J = imread('goldhill.jpg');
    psnr(i,3) = myPSNR(I,J);
    bpp(i,7) = cnt / size(I,1) / size(I,2) * 8;
    
    quality = quality / 2;
    quant = quant*3/2;

    figure();
    imshow(J);
end 

figure();
plot(bpp(:,2), psnr(:,1),'r',...
    bpp(:,3), psnr(:,1),'g',...
    bpp(:,5), psnr(:,1),'b',...
    bpp(:,6), psnr(:,2),'k',...
    bpp(:,7), psnr(:,3),'k--');
% plot(bpp(:,1), psnr(:,1),'b',...
%     bpp(:,2), psnr(:,1),'r',...
%     bpp(:,3), psnr(:,1),'g',...
%     bpp(:,4), psnr(:,1),'k',...
%     bpp(:,5), psnr(:,1),'b',...
%     bpp(:,6), psnr(:,2),'r--');
grid on;
xlabel('bpp');
ylabel('PSNR');
title('image compression');
%axis([0 1.2 25 36]); %goldhill
%axis([0 0.8 26 38]); %lena
axis([0 1.25 23 35]); %barbara
legend('кодирование без памяти',...
    'контекстное кодирование',...
    'знаковое контекстное кодирование',...
    'стандарт JPEG 2000',...
    'стандарт JPEG',...
    'Location','NorthWest');
% legend('256 models',...
%     '1 model',...
%     'context encoding',...
%     'sign context',...
%     'context for sign',...
%     'Location','NorthWest');
end
