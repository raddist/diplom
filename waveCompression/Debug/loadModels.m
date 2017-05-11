function [] = loadModels()

fid = fopen('model.bin', 'rb');
if fid == -1 
     error('File is not opened'); 
end

cnt=0;              % инициализация счетчика
models_size = 0;      % количество строк в массиве
max_size = 0;     % количество столбцов в массиве
[V,N] = fread(fid, 2,'uint');
model_size = V(1);
max_size = V(2);

arr = zeros(model_size, max_size);

for model = 1:model_size
    
    if (model == 1)
        cap = max_size;
    else
        [cap,N] = fread(fid, 1,'uint');
    end
    for j = 1:cap
        [V,N] = fread(fid, 1,'uint');
        if N > 0        % если элемент был прочитан успешно, то 
            arr(model, j) = V;
            cnt=cnt+1;  % увеличиваем счетчик на 1 
        end
    end
end

fclose(fid);

end