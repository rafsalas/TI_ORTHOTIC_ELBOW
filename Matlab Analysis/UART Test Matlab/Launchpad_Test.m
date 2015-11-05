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
fprintf(s,'A');

out_uni = fscanf(s)
out_native = unicode2native(out_uni)
out_hex = dec2hex(out_native,2)


%x = linspace(0,1,69);
%plot(x,out_byte) 

fclose(s)
delete(s)
clear s