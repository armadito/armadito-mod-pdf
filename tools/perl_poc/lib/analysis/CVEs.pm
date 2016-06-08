package CVEs;

use strict;

my $DEBUG = "no";

# This function detects
sub CVE_2010_2883_Detection{

	my $ref = shift;
	my %pdfObjects = %{$ref};
	my $fontfile;
	my $status = "none";

	print "\n\n:::CVE_2010_2883_Detection:::\n" unless $DEBUG eq "no";

	# Get font descriptors objects	
	my @objs = values(%pdfObjects);
	foreach(@objs){
	
		if( exists($_->{"type"}) && $_->{"type"} eq "/FontDescriptor" ){
			print "Found FontDescriptor object :: $_->{ref}\n" unless $DEBUG eq "no";
			
			if(exists($_->{"fontfile"}) && $_->{"fontfile"} =~ /(\d+\s\d\sR)/){
				$fontfile = $1 ;
				$fontfile =~ s/R/obj/;
				print "font File found :: $fontfile\n" unless $DEBUG eq "no";
			}else{
				next;
			}
			
			# Get the font file stream
			if(exists($pdfObjects{$fontfile}) && exists($pdfObjects{$fontfile}->{"stream_d"}) && length($pdfObjects{$fontfile}->{"stream_d"}) > 0 ){
			
				my $fontstream = $pdfObjects{$fontfile}->{"stream_d"};
				#print "font stream = $fontstream\n";
				
				# Check the length of the decoded stream /!\
				#my $realen = length();
				print "Lenght1 = ".$pdfObjects{$fontfile}->{"length1"}."\n" unless ($DEBUG eq "no" or ! exists($pdfObjects{$fontfile}->{"length1"})) ;
				print "Real length = ".length($fontstream)."\n" unless $DEBUG eq "no";
				if(exists($pdfObjects{$fontfile}->{"length1"})  && $pdfObjects{$fontfile}->{"length1"} != length($fontstream)){
					print "Warning :: Font File decoded stream Length is Wrong :: ".$pdfObjects{$fontfile}->{"length1"}." :: ".length($fontstream)."\n" unless $DEBUG eq "no";
					#$TESTS_CAT_3{"CVE_2010_2883"} = "BAD_FONT_FILE_LENGTH";
					$status = "BAD_FONT_FILE_LENGTH"; 
				}
				
				# Check TrueType required tables
				# - cmap - glyf - head - hhea - hmtx - loca - maxp - name - post
				# Detect the SING ()Smart INdependent Glyphlets) string
				if($fontstream =~ /SING/ ){
					print "Warning :: Found SING (Smart INdependent Glyphlets) :: Possible CVE_2010_2883\n" unless $DEBUG eq "yes";
					#$TESTS_CAT_3{"CVE_2010_2883"} = "DETECTED";
					$status = "DETECTED";
					
					# TODO combine with previous test (bad_font_file_length) to detect CVE
				}
				
						
			}else{
				print "Warning :: CVE_2010_2883_Detection :: Font File Object $fontfile is not defined :\n" unless $DEBUG eq "no";
			}
			
		}
	}

	return $status;
}

1;

__END__

