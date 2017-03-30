function [data size] = readBMP()

% open new file to analize encoding
fid = fopen('test2.bin', 'rb');
if fid == -1 
    error('File is not opened'); 
end

data=0;                % инициализаци€ переменной 
cnt=1;              % инициализаци€ счетчика 
while ~feof(fid)    % цикл, пока не достигнут конец файла 
    [V,N] = fread(fid, 1, 'uint8');  %считывание одного 
% значени€ double (V содержит значение 
% элемента, N Ц число считанных элементов) 
    if N > 0        % если элемент был прочитан успешно, то 
        data(cnt)=V;   % формируем вектор-строку из значений V 
        cnt=cnt+1;  % увеличиваем счетчик на 1 
    end 
end 

fclose(fid);        % закрытие файла
size = cnt;

end