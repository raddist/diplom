function arr = loadModels()

fid = fopen('model.bin', 'rb');
if fid == -1 
     error('File is not opened'); 
end

cnt=0;              % ������������� ��������
models_size = 0;      % ���������� ����� � �������
max_size = 0;     % ���������� �������� � �������
[V,N] = fread(fid, 2,'uint');
model_size = V(1);
max_size = V(2);

arr = zeros(model_size, max_size + 1);

for model = 1:model_size
    
    if (model == 1)
        cap = max_size;
    else
        [cap,N] = fread(fid, 1,'uint');
    end
    
    arr(model, 1) = cap;
    for j = 1:cap
        [V,N] = fread(fid, 1,'uint');
        if N > 0        % ���� ������� ��� �������� �������, �� 
            arr(model, j + 1) = V;
            cnt=cnt+1;  % ����������� ������� �� 1 
        end
    end
end

fclose(fid);

end