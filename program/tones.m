
outputfile="tones.h"

function o=tonetime(i)
o=floor(1/(440*2^(i/12))*1e6);
end



out=fopen(outputfile,'w');


function o=tone(i)
scale={ "c"; "cis"; "d"; "dis"; "e"; "f"; "fis"; "g"; "gis"; "a"; "ais"; "h" };

octave=floor(i/12);
tone=mod(i,12);

o=sprintf("#define %s%d %d\n", scale{tone+1},octave,tonetime(i+3));
end


for i = 0:96
fprintf(out,"%s",tone(i));
end

fclose(out);
