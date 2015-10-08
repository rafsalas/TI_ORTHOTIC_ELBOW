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
set(s,'BaudRate', 115200);
set(s, 'Parity', 'none');
set(s, 'DataBits', 8);
set(s, 'StopBit', 1);

%Open Serial Port
fopen(s);


%fprintf(s,'*IDN?')
out = fscanf(s)

fclose(s)
delete(s)
clear s