function [arr arr_height arr_width] = loadArrFromFile(file_name)

fid = fopen(file_name, 'rb');
if fid == -1 
     error('File is not opened'); 
end

cnt=0;              % инициализация счетчика
arr_height = 0;      % количество строк в массиве
arr_width = 0;     % количество столбцов в массиве
[V,N] = fread(fid, 2,'int');
arr_height = V(1);
arr_width = V(2);

arr = zeros(arr_height, arr_width);

while ~feof(fid)    % цикл, пока не достигнут конец файла 
    [V,N] = fread(fid, 1,'int');
    
    if N > 0        % если элемент был прочитан успешно, то 
        arr(floor(cnt/arr_height)+1, mod(cnt, arr_width)+1) = V;
        cnt=cnt+1;  % увеличиваем счетчик на 1 
    end
end

fclose(fid);

end
