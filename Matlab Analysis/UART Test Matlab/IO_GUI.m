%-------------------------------
%Assign s to Serial Port
s = serial('COM8');

fclose(s);
delete(s);
clear s

s=serial('COM8');

%Specify Settings
%Verify with Device Manager
set(s, 'InputBufferSize', 24);
set(s, 'FlowControl', 'none');
set(s, 'BaudRate', 115200);
set(s, 'Parity', 'none');
set(s, 'DataBits', 8);
set(s, 'StopBit', 1);

%Open Serial Port
fopen(s);


%-------------------------------
%Timer
%t = timer;
%t.StartFcn = @(~,thisEvent)disp([thisEvent.Type ' executed '...
%    datestr(thisEvent.Data.time,'dd-mmm-yyyy HH:MM:SS.FFF')]);
%t.TimerFcn = @(~,thisEvent)disp([thisEvent.Type ' executed '...
%     datestr(thisEvent.Data.time,'dd-mmm-yyyy HH:MM:SS.FFF')]);
%t.StopFcn = @(~,thisEvent)disp([thisEvent.Type ' executed '...
%    datestr(thisEvent.Data.time,'dd-mmm-yyyy HH:MM:SS.FFF')]);
%t.Period = 0.1;
%t.TasksToExecute = 30;
%t.ExecutionMode = 'fixedRate';
%start(t)

%-------------------------------
%Data
t=0:0.00025:1
x=sin(2*pi*30*t)+sin(2*pi*60*t);
y = decimate(x,4);

subplot 211
stem(0:120,x(1:121),'filled','markersize',3)
grid on
xlabel 'Sample number',ylabel 'Original'
subplot 212
stem(0:30,y(1:31),'filled','markersize',3)
grid on
xlabel 'Sample number',ylabel 'Decimated'


%-------------------------------
%UART Transmit
fprintf(s,'A');



%-------------------------------
%UART Receive
for i = 0:5
    out_uni = fscanf(s)
    %out_native = unicode2native(out_uni)
    %out_hex = dec2hex(out_native,2)
end
%-------------------------------

fclose(s)
delete(s)
clear s