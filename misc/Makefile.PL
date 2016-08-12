open F, ">", "Makefile";
print F "all: ";
for my $c (<*.c>) {
	my $exe = $c;
	$exe =~ s/\.c$/.exe/;
	print F "$exe ";
}
print F "\n\n";

for my $c (<*.c>) {
	my $exe = $c;
	$exe =~ s/\.c$/.exe/;
	print F "$exe: $c\n\tcl $c user32.lib shell32.lib ole32.lib\n\n";
}
print F "clean:\n\trm -f *.exe *.obj";
close F;
