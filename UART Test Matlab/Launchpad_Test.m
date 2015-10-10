%Assign s to Serial Port
s = serial('COM9');

fclose(s);
delete(s);
clear s

s=serial('COM9');

%Specify Settings
%Verify with Device Manager
set(s, 'InputBufferSize', 244);
set(s, 'FlowControl', 'none');
set(s, 'BaudRate', 115200);
set(s, 'Parity', 'none');
set(s, 'DataBits', 8);
set(s, 'StopBit', 1);

%Open Serial Port
fopen(s);

%Prompt Data Stream
%fprintf(s,'%c','A');

out_uni = fscanf(s)
out_byte = unicode2native(out_uni)%,'Hex') 

fclose(s)
delete(s)
clear s