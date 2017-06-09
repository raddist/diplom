function PSNRshell(psnr, bpp, img_name)
close all
% bpp(:,2), psnr(:,1),'r',...   '����������� ��� ������',...
% bpp(:,1), psnr(:,1),'y',...   '����������� � �������',...
% bpp(:,3), psnr(:,1),'g',...   '����������� �����������',...
% bpp(:,5), psnr(:,1),'b',...   '�������� ����������� �����������',...
% bpp(:,6), psnr(:,2),'m',...   '�������� JPEG 2000',...
% bpp(:,7), psnr(:,3),'k');   '�������� JPEG',...

% grid on;
% xlabel('bpp,���');
% ylabel('PSNR,��');
% title('���������� ��� ����������� Goldhill');
% %axis([0 1.2 25 36]); %goldhill
% %axis([0 0.8 26 38]); %lena
% %axis([0 1.25 23 35]); %barbara
% legend(
%     'Location','SouthEast');

%axis([0 1.2 25 36]); %goldhill
%axis([0 0.8 26 38]); %lena
%axis([0 1.25 23 35]); %barbara

current_axis = [];
if strcmp(img_name, 'Goldhill')
    current_axis = [0 1.2 25 36]; %goldhill
end
if strcmp(img_name, 'Lena')
    current_axis = [0 0.8 26 38]; %lena
end
if strcmp(img_name, 'Barbara')
    current_axis = [0 1.25 23 35]; %barbara
end
if strcmp(img_name, 'Boat')
    current_axis = [0 1.5 0 40]; %boat
end

%% JPEG vs JPEG 2000
figure();
plot(bpp(:,6), psnr(:,2),'m',...
     bpp(:,7), psnr(:,3),'k','LineWidth',2);
grid on;
xlabel('bpp,���','FontSize',14);
ylabel('PSNR,��','FontSize',14);

title(strcat('���������� ��� �����������',32,strcat(img_name)),'FontSize',14);

axis(current_axis); %goldhill

h_legend = legend('�������� JPEG 2000',...
    '�������� JPEG',...
    'Location','SouthEast');
set(h_legend,'FontSize',14);

%% JPEG vs arifmetic
figure();
plot(bpp(:,2), psnr(:,1),'r',...
     bpp(:,7), psnr(:,3),'k','LineWidth',2);
grid on;
xlabel('bpp,���','FontSize',14);
ylabel('PSNR,��','FontSize',14);

title(strcat('���������� ��� �����������',32,strcat(img_name)),'FontSize',14);

axis(current_axis); %goldhill

h_legend = legend('����������� ��� ������',...
    '�������� JPEG',...
    'Location','SouthEast');
set(h_legend,'FontSize',14);

%% JPEG vs arifmetic vs mem arifm
figure();
plot(bpp(:,1), psnr(:,1),'y',...
     bpp(:,2), psnr(:,1),'r',...
     bpp(:,7), psnr(:,3),'k','LineWidth',2);
grid on;
xlabel('bpp,���','FontSize',14);
ylabel('PSNR,��','FontSize',14);

title(strcat('���������� ��� �����������',32,strcat(img_name)),'FontSize',14);

axis(current_axis); %goldhill

h_legend = legend('����������� � �������',...
    '����������� ��� ������',...
    '�������� JPEG',...
    'Location','SouthEast');
set(h_legend,'FontSize',14);

%% JPEG vs mem arifm vs context
figure();
plot(bpp(:,3), psnr(:,1),'g',...
     bpp(:,1), psnr(:,1),'y',...
     bpp(:,7), psnr(:,3),'k','LineWidth',2);
grid on;
xlabel('bpp,���','FontSize',14);
ylabel('PSNR,��','FontSize',14);

title(strcat('���������� ��� �����������',32,strcat(img_name)),'FontSize',14);

axis(current_axis); %goldhill

h_legend = legend('����������� �����������',...
    '����������� � �������',...
    '�������� JPEG',...
    'Location','SouthEast');
set(h_legend,'FontSize',14);

%% context vs sign context
figure();
plot(bpp(:,5), psnr(:,1),'b',...
     bpp(:,3), psnr(:,1),'g','LineWidth',2);
grid on;
xlabel('bpp,���','FontSize',14);
ylabel('PSNR,��','FontSize',14);

title(strcat('���������� ��� �����������',32,strcat(img_name)),'FontSize',14);

axis(current_axis); %goldhill

h_legend = legend('�������� ����������� �����������',...
    '����������� �����������',...
    'Location','SouthEast');
set(h_legend,'FontSize',14);

%% compile all the tests
figure();
plot(bpp(:,6), psnr(:,2),'m',...
     bpp(:,5), psnr(:,1),'b',...
     bpp(:,3), psnr(:,1),'g',...
     bpp(:,1), psnr(:,1),'y',...
     bpp(:,2), psnr(:,1),'r',...
     bpp(:,7), psnr(:,3),'k','LineWidth',2);
grid on;
xlabel('bpp,���','FontSize',14);
ylabel('PSNR,��','FontSize',14);

title(strcat('���������� ��� �����������',32,strcat(img_name)),'FontSize',14);

axis(current_axis); %goldhill

h_legend = legend('�������� JPEG 2000',...
    '�������� ����������� �����������',...
    '����������� �����������',...
    '����������� � �������',...
    '����������� ��� ������',...
    '�������� JPEG',...
    'Location','SouthEast');
set(h_legend,'FontSize',14);
end

