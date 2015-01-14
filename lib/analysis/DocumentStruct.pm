package DocumentStruct;

use strict;
use MIME::Base64 ();

my $DEBUG = "no";


# Check the magic number of a PDF file 
sub CheckMagicNumber{

	my $file_ref= shift;
	my $file = $file_ref;
	
	my $len=8;
	my $offset=0;
	my $ver="undef";
	
	seek ($file, 0, 0);
	read $file, $ver, $len, $offset or print "read failed :: $!\n";
	if( $ver =~ /\%PDF-\d\.\d/){
		print "PDF header : OK\n" unless $DEBUG eq "no";
		return ($ver,"OK");
	}
	
	# Check string <?xml version="1.0"?><?xfa ?><xdp:xdp xmlns:xdp="http://ns.adobe.com/xdp/"><pdf xmlns="http://ns.adobe.com/xdp/pdf/"><document><chunk>
	seek ($file, 0, 0); 	# rewind file
	my $content = do { local $/; <$file>};
	#print "$content\n";
	if($content =~ /<xdp:xdp\sxmlns:xdp=\"http:\/\/ns\.adobe\.com\/xdp\/"><pdf\sxmlns=\"http:\/\/ns\.adobe\.com\/xdp\/pdf\/\"><document><chunk>(.*)<\/chunk><\/document><\/pdf>/si){
	#if($content =~ /<chunk>(.*)<\/chunk>/si){
		print "This document is an XML Data Package (XDP)\n" unless $DEBUG eq "no";
		my $chunkContent = $1;
		#print "chunkContent = $chunkContent\n";
		
		#decode base64 content
		my $decodedContent = MIME::Base64::decode($chunkContent) or print "Error while decoding base64 :: $!\n";
		#print "decoded content = $decodedContent\n";
		
		# write content in a new file
		close($file);
		open $file, ">tmp.pdf" or die "open failed in tmp.pdf : $! ";
		binmode $file;
		print $file $decodedContent;
		#print "file handle = $file\n";
		close($file);
		open $file, "<tmp.pdf" or die "open failed in tmp.pdf : $! ";
		binmode $file;
		#print "file handle 2 = $file\n";
		#print "$file;
		
		seek ($file, 0, 0);
		read $file, $ver, $len, $offset or print "read failed :: $!\n";
		
		if( $ver =~ /\%PDF-\d\.\d/){
			print "PDF header : OK\n" unless $DEBUG eq "no";
			return ($ver,"XDP_FILE");
		}
		
		return ($ver,"BAD_MAGIC");
		
		
	}
	
	return ($ver, "BAD_MAGIC");
}


# This function check if the pdf document is empty with only active content (js,embedded_file,openaction etc.)
sub Empty_Pages_Document_detection{

	my $ref = shift;
	
	#print "DEBUG = $ref\n";
	
	my %pdfObjects = %{$ref};
	
	
	my $ret=0;
	my $numPages =0; # Number of pages found
	my $active_content =0; # Number of js, embedded files
	
	print "\n\n ::: Empty Pages With Active Content detection ::: \n" unless $DEBUG eq "no";
	
	my @objs = values(%pdfObjects);
	foreach(@objs){
	
		
		if( exists($_->{"type"}) && $_->{"type"} eq "/Pages" ){
		
			print "FOUND Pages object :: $_->{ref} :: \n" unless $DEBUG eq "no";
			
			# Get kid node pages
			my @pages = $_->{"kids"} =~ /(\d+\s\d\sR)/sg;
			#print @pages;
				
			foreach(@pages){
				my $page_ref = $_;
				$page_ref =~ s/R/obj/;
				#print "page ref = $page_ref\n";
				
				# if the page exists and the /Content parameter is set
				if(exists($pdfObjects{$page_ref}) && exists($pdfObjects{$page_ref}->{"pagecontent"})  ){
				
					# Check if it's not an empty content
					#my $p_content = $pdfObjects{$page_ref}->{"pagecontent"};
					
					
					# If the Contents fiels is an array
					my @pcontents = $pdfObjects{$page_ref}->{"pagecontent"} =~ /(\d+\s\d\sR)/sg;
					
					foreach (@pcontents){
					
						my $content_page_obj = $_;
						$content_page_obj =~ s/R/obj/;
						
						#print ":: page content = $content_page_obj ::\n";#" $pdfObjects{$contentp}->{content}\n";
						
						if(exists($pdfObjects{$content_page_obj}) && exists($pdfObjects{$content_page_obj}->{"stream"}) && length($pdfObjects{$content_page_obj}->{"stream"}) > 0  ){
							$ret ++;
							print "Page $page_ref is not empty => OK\n"unless $DEBUG eq "no";
						
						}elsif(! exists($pdfObjects{$content_page_obj})){
							print "Warning : Content Object ($content_page_obj) of page $page_ref doesn\'t exist\n" unless $DEBUG eq "no";
							
						}elsif( exists($pdfObjects{$content_page_obj}->{content}) ){
						
							# Trigger the case when the object represents an array of objects Ex: [422 0 R 423 0 R 424 0 R 425 0 R 426 0 R 427 0 R 428 0 R 429 0 R]
							
							my @content_page_array = $pdfObjects{$content_page_obj}->{"content"} =~ /(\d+\s\d\sR)/sg;
							
							foreach(@content_page_array){
								
								my $content_page_obj_2 = $_;
								$content_page_obj_2 =~ s/R/obj/;
								#print " Found obj :: $content_page_obj_2\n";
								
								if(exists($pdfObjects{$content_page_obj_2})){
								
									if( exists($pdfObjects{$content_page_obj_2}->{"stream"}) && length($pdfObjects{$content_page_obj_2}->{"stream"}) > 0 ){
										$ret ++;
										print "Found content of the page $page_ref in obj $content_page_obj_2 => OK\n"unless $DEBUG eq "no";
									}else{
										print "Warning :: Page content Object ($content_page_obj_2) is empty\n" unless $DEBUG eq "no";
									}
								
								}else{
									print "Warning :: Page content Object ($content_page_obj_2) is not defined\n" unless $DEBUG eq "no";
								}	
							}
							
						
						}else{
							print "Warning : The Stream of the Content Object is empty\n" unless $DEBUG eq "no";
							
						}	
					
					}
				
					
				}elsif(! exists($pdfObjects{$page_ref})){
					print "Warning : Page $page_ref does\'nt exist.\n" unless $DEBUG eq "no";
				}else{
					print "Warning : Page $page_ref is empty\n" unless $DEBUG eq "no";
				}
				
			
			}
			
		}
		
		# TODO Verify that the number of treated pages is the number of pages in the document.
	
	}
	
	return $ret;
	
}





# This function check if the xref table is conform
# TODO return 0 if failed and 1 if sucess and the error status
sub Check_xref{
	
	my ($trailer, $fh, $pdfObjects_ref) = @_;
	my $xref_offset;
	my $len=4; # "xref" string length.
	my $res;
	my $ret = 0;
	
	my %pdfObjects = %{$pdfObjects_ref};

	# Get the startxref offset in the trailer
	if ($trailer =~ /startxref\s*(\d+)\s*%%EOF/){
		$xref_offset = $1;
	}else{
		#return (0,$BAD_XREF_OFFSET);
		return 0;
	}
	print "\nxref_offset = $xref_offset\n" unless $DEBUG eq "no";


	# Test XRef keyword
	seek ($fh, $xref_offset, 0); # Go to the xref offset
	read ($fh, $res, $len) or print "Check_xref :: read failed :: $!\n";
	print "res = $res\n" unless $DEBUG eq "no";

	
	if($res ne "xref"){ # Test for object stream reference
		$len = 10;
		seek ($fh, $xref_offset, 0); # Go to the xref offset
		read ($fh, $res, $len) or print "Check_xref :: read failed :: $!\n";
		print "res2 = $res\n" unless $DEBUG eq "no";

		if($res =~ /^(\d+\s\d\sobj)/){
			# TODO decode xref stream.
			#print "";
			# Check if the object is well a XRef type object
			my $obj_ref= $1;
			
			if(exists($pdfObjects{$obj_ref}) && $pdfObjects{$obj_ref}->{"type"} eq "/XRef"){
				return 1;
			}else{
				return 0;
			}
			
			
		}else{
			#print "BAD xref offset!!\n";
			#return $BAD_XREF_OFFSET;
			#return (0,$BAD_XREF_OFFSET);
			return 0;
		}

	}

	# Get xref entries
	my $xref_content=$res;
	#print "Offset position = ".tell($fh)."\n" unless $DEBUG eq "no";
	my $i=5;
	while(!( $xref_content =~ /trailer$/)){
		
		read ($fh, $xref_content, 1, $i) or print "Check_xref :: read failed :: $!\n";
		$i++;	
	}

	print "$xref_content\n" unless $DEBUG eq "no";

	# nnnnnnnnnn ggggg n eol
	# nnnnnnnnnn is a 10-digit byte offset
	# ggggg is a 5-digit generation number
	# n is a literal keyword identifying this as an in-use entry
	# my @xref_entries = $xref_content =~ /(\d{10}\s\d{5}\s[f|n]\n)/;
	my $first_obj;
	my $number_of_entries;
	if($xref_content =~ /(\d{1,3})\s(\d{1,3})/g){
		$first_obj = $1;
		$number_of_entries=$2;
		print "$first_obj :: $number_of_entries\n\n" unless $DEBUG eq "no";
	}
	my @xref_entries = $xref_content =~ /(\d{10}\s\d{5}\s[f|n])/g;

	# @pdf_objects;

	# Check object's offets
	my $id=0;
	foreach(@xref_entries){
		
		if(/(\d{10})\s(\d{5})\s([f|n])/){

			#print "\n$1::$2::$3\n";
			my $off = $1;
			my $gen = $2;
			my $free = $3;

			$len = 8; # TODO calc len en fonction du nombre d'entrées.
			seek ($fh, $off, 0);
			read ($fh, $res, $len) or print "Check_xref :: read failed :: off=$off :: len=$len\n";
			chomp $res;
			#print "res = $res\n";

			if($res =~/($id\s0\sobj)/ or $free ne "n"){
			
				my $obj_ref = $1;	
				
				# save the object's offset
				if(exists($pdfObjects{$obj_ref}) ){
					print "object $obj_ref is at offset $off\n" unless $DEBUG eq "no";
					$pdfObjects{$obj_ref}->{"offset"} = $off ;
				}
				
			
			}else{
				print "WRONG Object offset :: $id $gen obj :: offset $off\n"unless $DEBUG eq "yes";
				#$ret = $BAD_OBJ_OFFSET;
				#return (0,$BAD_OBJ_OFFSET);
				return 0;
			}
			$id ++;

		}
	}
	

	return 1;
}





1;
__END__
