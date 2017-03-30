function [data size] = readBMP()

% open new file to analize encoding
fid = fopen('test2.bin', 'rb');
if fid == -1 
    error('File is not opened'); 
end

data=0;                % ������������� ���������� 
cnt=1;              % ������������� �������� 
while ~feof(fid)    % ����, ���� �� ��������� ����� ����� 
    [V,N] = fread(fid, 1, 'uint8');  %���������� ������ 
% �������� double (V �������� �������� 
% ��������, N � ����� ��������� ���������) 
    if N > 0        % ���� ������� ��� �������� �������, �� 
        data(cnt)=V;   % ��������� ������-������ �� �������� V 
        cnt=cnt+1;  % ����������� ������� �� 1 
    end 
end 

fclose(fid);        % �������� �����
size = cnt;

end