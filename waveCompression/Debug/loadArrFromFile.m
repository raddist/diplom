function [arr arr_height arr_width] = loadArrFromFile(file_name)

fid = fopen(file_name, 'rb');
if fid == -1 
     error('File is not opened'); 
end

cnt=0;              % ������������� ��������
arr_height = 0;      % ���������� ����� � �������
arr_width = 0;     % ���������� �������� � �������
[V,N] = fread(fid, 2,'int');
arr_height = V(1);
arr_width = V(2);

arr = zeros(arr_height, arr_width);

while ~feof(fid)    % ����, ���� �� ��������� ����� ����� 
    [V,N] = fread(fid, 1,'int');
    
    if N > 0        % ���� ������� ��� �������� �������, �� 
        arr(floor(cnt/arr_height)+1, mod(cnt, arr_width)+1) = V;
        cnt=cnt+1;  % ����������� ������� �� 1 
    end
end

fclose(fid);

end
