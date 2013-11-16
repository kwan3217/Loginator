printf("const fp sinTable[]={\n");
my $pi=3.1415926535897932;
for(my $i=0;$i<=900;$i++) {
  printf("/*%05.1f*/  %10.8f%s\n",$i/10.0,sin($i/10.0*$pi/180.0),$i==900?"":",");
}
printf("};\n")