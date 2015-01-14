package CleanRewriting;

use strict;

use lib::utils::Filters;
use File::Basename;

#use bytes;

my $DEBUG = "no";


# This function remove the JavaScript content of an object (not an object stream)
sub RemoveJSContentFromObj{

	my ($obj,$pdfObjects) = @_;
	
		
	print "The object is at offset $pdfObjects->{$obj}->{objStmOff} in object stream\n";	
		
		
	my $len = length($pdfObjects->{$obj}->{js});
		
	print "js len = $len\n";
	
	
	my $comment = "";
	my $pat= $pdfObjects->{$obj}->{content};
	for(my $i =0; $i <$len-2 ; $i++){
		$comment.=" ";
	}
	$comment="(".$comment.")";
		
	# Get the offset of the js content
	my $off = index($pdfObjects->{$obj}->{content}, $pdfObjects->{$obj}->{js});
		
		
	print "verif1 :: $pdfObjects->{$obj}->{content}\n";
	
	# Replace js content by empty string
	substr($pdfObjects->{$obj}->{content},$off,$len,$comment);
	
	print "verif2 :: $pdfObjects->{$obj}->{content}\n";
		
	return 0;			
}

sub RemoveJSContentFromXFA{

	my ($obj,$pdfObjects) = @_;
	
	#print " Content = $pdfObjects->{$obj}->{stream_d}\n\n" if $obj eq "26 0 obj";
	
	# <script contentType="application/x-javascript"></script>
	# 
	#my @js_content = $pdfObjects->{$obj}->{stream_d} =~ /(javascript)/gi ;
	#my @js_content = $pdfObjects->{$obj}->{stream_d} =~ /(<script contentType="application\/x-javascript"\s*>(.*)<\/script\s*>)/gi ; 
	#my @js_content = $pdfObjects->{$obj}->{stream_d} =~ /(<script contentType="application\/x-javascript"\s*>)/gi ; 
	#my @js_content = $pdfObjects->{$obj}->{stream_d} =~ /(<script contentType="application\/x-javascript"\s*>(.*)<\/script\s*>)/ig;
	#my @js_content = $pdfObjects->{$obj}->{stream_d} =~ /(<script contentType="application\/x-javascript"\s*>(.*?)<\/script)/sig;
	#my @js_content = $pdfObjects->{$obj}->{content} =~ /(<script contentType="application\/x-javascript"\s*>.*?<\/script\s*>)/sig;
	my @js_content; # = $pdfObjects->{$obj}->{content} =~ /(<script contentType="application\/x-javascript"\s*>.*?<\/script\s*>)/sig;
	
	if( exists($pdfObjects->{$obj}->{"stream_d"})){
		#print "stream_d == $pdfObjects->{$obj}->{stream_d} \n";
		@js_content = $pdfObjects->{$obj}->{"stream_d"} =~ /(<script contentType="application\/x-javascript"\s*>.*?<\/script\s*>)/sig;
	}else{
		@js_content = $pdfObjects->{$obj}->{"stream"} =~ /(<script contentType="application\/x-javascript"\s*>.*?<\/script\s*>)/sig;
	}
	
		
	print "js_content = $#js_content\n";
	
	if($#js_content < 0 ){
		print "Error :: RemoveJSContentFromXFA :: No JavaScript balise found in XFA form\n";
		return -1;
	}
	
	foreach(@js_content){
		
		print "JS_CONTENT == $_\n\n";
		
		# locate de content in the stream
		#my $off = index($pdfObjects->{$obj}->{$content}, $pdfObjects->{$obj}->{js});
		
		if( exists($pdfObjects->{$obj}->{"stream_d"})){
		
			# TODO if the object is encoded
			print "TODO if the stream is encoded\n";
			my $old_content = $pdfObjects->{$obj}->{content};
			
			$pdfObjects->{$obj}->{"stream_d"} =~ s/\Q$_\E//sig;
			
			my $stream = &Filters::FlateEncode($pdfObjects->{$obj}->{"stream_d"});
			
			$pdfObjects->{$obj}->{content} = $pdfObjects->{$obj}->{"ref"}."\r<<".$pdfObjects->{$obj}->{"dico"}.">>stream"."\r\n".$stream."\r\nendstream\rendobj";
			
		}else{
		
			my $old_content = $pdfObjects->{$obj}->{content};
			$pdfObjects->{$obj}->{content} =~ s/\Q$_\E//sig;

		}

		#if($old_content eq $pdfObjects->{$obj}->{content}){
		#	print "ERR :: No modification on content :: $_ :: $old_content\n";
		#}
		

	}
	
	
	

}


# This function remove an object given in parameter from an object stream
sub RemoveObjectFromObjStream{

	my ($objStm,$obj,$pdfObjects) = @_;
	
	
	if(exists($pdfObjects->{$objStm}->{stream_d})){
	
		print "The object is at offset $pdfObjects->{$obj}->{objStmOff} in object stream\n";
		
			
		my $len = length($pdfObjects->{$obj}->{js});
		
		print "len = $len\n";
		
		my $comment = "";
		my $pat= $pdfObjects->{$obj}->{content};
		for(my $i =0; $i <$len-2 ; $i++){
			$comment.=" ";
		}
		$comment="(".$comment.")";
		my $com_len = length($comment);
		
		my $js = $pdfObjects->{$obj}->{js};
		#print "Replacing :: $js :: in :: $pat :: by :: $comment\n";
		
		# search the offset of the js code
		my $off = index($pdfObjects->{$objStm}->{stream_d}, $js ,$pdfObjects->{$obj}->{objStmOff} );
		
		print "js to replace = $pdfObjects->{$obj}->{js} :: $com_len \n";
		my $verif = substr($pdfObjects->{$objStm}->{stream_d},$off,$len );
		print "verif :: to replace  = $verif\n";
		$verif = substr( $pdfObjects->{$objStm}->{stream_d}, $pdfObjects->{$obj}->{objStmOff}, length($pdfObjects->{$obj}->{content}) );
		print "verif2 :: rplace in  = $verif\n";
		
		
		my $length_d = length($pdfObjects->{$objStm}->{stream_d});
		# Comment the content by =>  (//)
		substr($pdfObjects->{$objStm}->{stream_d},$off,$len,$comment);
		
		#$off = index($pdfObjects->{$obj}->{content}, $js);
		# 
		#substr($pdfObjects->{$obj}->{content},$off,$len,$comment);
		
		substr($pdfObjects->{$objStm}->{stream_d},$pdfObjects->{$obj}->{objStmOff},$len,$comment);


		
		$verif = substr($pdfObjects->{$objStm}->{stream_d},$pdfObjects->{$obj}->{objStmOff},length($pdfObjects->{$obj}->{content}) );
		print "verif3 :: res = $verif\n";
		print "verifx :: length_d = $length_d :: length_after ".length($pdfObjects->{$objStm}->{stream_d})."\n" if ($length_d != length($pdfObjects->{$objStm}->{stream_d}));
		# Apply filter mention in the content
		
		# Get filters
		print " Filters  :: :$pdfObjects->{$objStm}->{filters}:\n";
		if($pdfObjects->{$objStm}->{"filters"}  =~ /FlateDecode/){
		
			print "Encode stream_d with Flate encoding\n";
			
			###################
			#my $test = &Filters::FlateEncode("Hello");
			#my $test2 = &Filters::FlateEncode("He  o");
			
			#print "DEBUG_TEST :: $test :: $test2 :: ".length($test)." :: ".length($test2)."\n";
			###################
			
			my $stream = &Filters::FlateEncode($pdfObjects->{$objStm}->{stream_d});
			my $oldstream = $pdfObjects->{$objStm}->{stream};
			$pdfObjects->{$objStm}->{stream} = $stream; 
			
			print " verif4 :: stream length old : new ".length($oldstream)." :: ".length($stream)."\n";
						
			
			#replace stream in content;
			#$pdfObjects->{$objStm}->{stream} = s/\Q$oldstream\E/$stream\n/s;
			
			#$pdfObjects->{$objStm}->{content} =~ s///;
			
			# Get index of stream in content
#			my $old_content = $pdfObjects->{$objStm}->{content} ;
#			my $ind = bytes::index ($pdfObjects->{$objStm}->{content}, $pdfObjects->{$objStm}->{stream}, $pdfObjects->{$objStm}->{offset});
#			print "verif z :: $ind ".substr ($pdfObjects->{$objStm}->{content} , $ind, bytes::length($pdfObjects->{$objStm}->{stream}))."\n";
#			substr ($pdfObjects->{$objStm}->{content} , $ind, bytes::length($pdfObjects->{$objStm}->{stream}), $stream);
#			
#			my $diff = length($stream) - length($pdfObjects->{$objStm}->{stream});
#			if($diff > 0){
#				my @objs = values(%{$pdfObjects});
#				#my $obj_off = $pdfObjects->{$objStm}->{offset};
#				foreach(@objs){
#					if(exists($_->{offset}) && $_->{offset} > $pdfObjects->{$objStm}->{offset} ){
#						$_->{offset}+=($diff+1);
#					}
#				}
#			}
			
			
			
			my $content = $pdfObjects->{$objStm}->{content};
			
			# Build new content
			$pdfObjects->{$objStm}->{content} = $pdfObjects->{$objStm}->{"ref"}."\r<<".$pdfObjects->{$objStm}->{"dico"}.">>stream"."\r\n".$stream."\r\nendstream\rendobj";
			
		
			#my $diff = bytes::length($stream) - bytes::length($oldstream);
			my $diff = bytes::length($pdfObjects->{$objStm}->{content}) - bytes::length($content);
			print "length new = ".length($pdfObjects->{$objStm}->{content})." :: ".length($content)." :: diff = $diff\n";
			if($diff > 0){
				my @objs = values(%{$pdfObjects});
				#my $obj_off = $pdfObjects->{$objStm}->{offset};
				foreach(@objs){
					if(exists($_->{offset}) && $_->{offset} > $pdfObjects->{$objStm}->{offset} ){
						$_->{offset}+=($diff);
					}
				}
			}
				
				
		}else{
			print "ERROR :: RemoveObjectFromObjStream :: another filter used :: $pdfObjects->{$objStm}->{filters} \n";
		}
		
		
		
	}
	
	
	
	
	
}



# this function remove or modify from the list all potentially dangerous objects
sub RemoveModifyDangerousObjects{

	my @to_analyse;
	my $active_content =0;
	
	#my (%js, %ef, %xfa ); # javascript , embedded files, xfa
	my ($js, $ef, $xfa ) = (0,0,0); # javascript , embedded files, xfa
	
	my $pdfObjects = shift;
	my @objs = values(%{$pdfObjects});
	
	foreach(@objs){
	
		# remove javascript 
		if( exists($_->{"js"}) or exists($_->{"javascript"}) or exists($_->{"js_obj"}) ){
		
			print "Warning :: RemoveModifyDangerousObjects :: Found javascript in  $_->{ref} :: \n" unless $DEBUG eq "no";
			
			# case if the javascript is described in another object
			if(exists($_->{"js_obj"})){
			
				# get the object
				my $js_obj = $_->{"js_obj"};
				$js_obj =~ s/R/obj/;
				print "Deleting javascript content of object = $js_obj :: \n";
				
				# erase content
				$pdfObjects->{$js_obj}->{content} = "$js_obj\nendobj";
				
				
			}elsif(exists($_->{"js"})){ # if the js is a string
				
				print "javascript content :: $_->{js} \n" unless $DEBUG eq "no";
				
				# If the object is packed in an object stream
				if( exists($_->{"objStm"})  ){
					
					print "javascript object $_->{ref} in Object stream $_->{objStm}\n" unless $DEBUG eq "no";
					
					# Remove the object in object stream
					#&RemoveObjectFromObjStream($_->{objStm},$_->{"ref"},$pdfObjects);
					&RemoveJSContentFromObj($_->{"ref"}, $pdfObjects);

				}else{
					$pdfObjects->{$_->{"ref"}}->{content} =~ s/\Q$_->{js}\E//;
				}
				
			}
			
			#print "Warning :: Active_Contents :: Found javascript in  $_->{ref} :: \n" unless $DEBUG eq "yes";
			#print "content = $_->{content}\n";
			$js ++;
			$active_content ++;
			print "\n\n";
		}
		
		if( exists($_->{"type"}) && $_->{"type"} eq "/EmbeddedFile" ){
			#print "Warning :: Found EmbeddedFile in $_->{ref}\n" unless $DEBUG eq "yes";
			$ef ++;
			$active_content ++;
			#print "\n\n";
		}
		
		
		# XFA processing
		if(exists($_->{"xfa"}) ){
			
			# an array of object
			my @xfas = $_->{"xfa"} =~ /(\d+\s\d\sR)/sg;
			
			#print @xfas;
			
			foreach (@xfas){
			
				my $xfa = $_;
				$xfa =~ s/R/obj/;
				print "found XFA obj :: $xfa\n" unless $DEBUG eq "no";
				
				if(exists($pdfObjects->{$xfa})){
				
					#print "found XFA obj :: $xfa\n";
					if(exists($pdfObjects->{$xfa}->{"stream_d"}) && length($pdfObjects->{$xfa}->{"stream_d"})>0 ){
						
						# Search javascript content
						# <script contentTyp='application'contentType='application/x-javascript'>
						if($pdfObjects->{$xfa}->{"stream_d"} =~ /(javascript)/si){
							
							print "Warning :: $1 :: RemoveModifyDangerousObjects :: Found javaScript in XFA (stream_d): $xfa\n" unless $DEBUG eq "no";
							$active_content ++;
							&RemoveJSContentFromXFA($xfa,$pdfObjects);
						}
						
						
					}elsif(exists($pdfObjects->{$xfa}->{"stream"}) && length($pdfObjects->{$xfa}->{"stream"})>0){
					
						if($pdfObjects->{$xfa}->{"stream"} =~ /javascript/si){
							print "Warning :: RemoveModifyDangerousObjects :: found javaScript in XFA (stream):: $xfa\n" unless $DEBUG eq "no";
							$active_content ++;
							&RemoveJSContentFromXFA($xfa,$pdfObjects);
						}
					}
				}
				
			}
		}
		
	}
	
	#if($active_content > 0){
		#$TESTS_CAT_1{"Active Content"} = $active_content;
	#}

	
	
	return $active_content;

}

# This function clean all javascript and suspiscious embedded files in the pdf.
sub Rewrite_clean{


	#my $filename = shift;
	my ($filename, $version, $pdfObjects, @trailers) = @_;
	#my %pdfObjects = %{$pdfObjs_ref};
	my @xref_table;
	
	#$filename ="clean.pdf";
	my $clean_pdf;
	
	my $clean_filename =basename($filename);
	#$clean_filename =~ s/\.pdf//;
	#$clean_filename .= "_clean.pdf";
	
	$clean_filename = "Cleaned_PDF/clean_".$clean_filename;
	print "cleaned file = $clean_filename\n";

	# Create the clean file
	open ($clean_pdf, ">$clean_filename" ) or die "Rewrite_clean :: failed to open file :: filename\n";

	# write header
	print $clean_pdf "$version\n";
	print $clean_pdf "%äüöß\n"; # add binary data for PDF interpretation
	
	
	&RemoveModifyDangerousObjects($pdfObjects);
	
	# TODO Remove dangerous embedded files (executables);
	
	
	my @objs = values(%{$pdfObjects});
	
	
	#print "......\n";
	
	my $high = 0;
	my $root = 0;
	
	# Write objects extracted from object stream
	foreach(@objs){
	
		my $num = -1;
		my $gen = 0;
		if($_->{ref} =~ /(\d+)\s(\d)\sobj/){
		
			$num =$1;
			$gen = $2;
			if( $1 > $high){
				$high = $1;
				
			}
		}
		
		if(exists($_->{type}) && $_->{type} eq "/Catalog"){
			$root = $_->{"ref"};
		}
		
		if(exists($_->{type}) && $_->{type} eq "/XRef"){
			next;
		}
		
		if(exists($_->{type}) && $_->{type} eq "/ObjStm"){
			next;
		}
		
#		if(exists($_->{type}) && $_->{type} eq "/Metadata"){
#			next;
#		}
#		
#		if(exists($_->{type}) && $_->{type} eq "/Info"){
#			next;
#		}
		
		# TODO rewrite the Info object
		# modify metadata		
		

		# Reach the end of the file
		seek($clean_pdf,0,2);
		my $offset = tell($clean_pdf);
		print "writing object $_->{ref} at ".tell($clean_pdf)."\n" unless $DEBUG eq "no";
		
		
		my $xref = sprintf("%010d",$offset)." ".sprintf("%05d",$gen)." n";
		$xref_table[$num+1] = $xref;
			
		
		# rebuild content
		if(exists($_->{objStm})){
			$_->{"content"} = $_->{"ref"}."\r".$_->{"content"}."\rendobj";
		}
				
		print $clean_pdf $_->{"content"};
		print $clean_pdf "\n";
		
		
	}
	
	# Go to the end of the file
	seek($clean_pdf,0,2);
	
	
	$root =~ s/obj/R/;
	$high = $high+1;
	
	# Write the XRef
	$xref_table[0] = "0 $high";
	my $offset = 0;
	my $gen = 65535;
	$xref_table[1] = "0000000000 65535 f";
	my $xref_offset = tell($clean_pdf);
	print $clean_pdf "xref\n";
	
	foreach(@xref_table){
	
		if($_){
			print $clean_pdf "$_\n";
			#print "$_\n";
		}else{
			print $clean_pdf "0000000000 65535 f\n";
			#print "0000000000 65535 f\n";
		}
		
		
	}
	
	
	# Write the trailer at the end of the file
	#$high = $high+1;
	my $trailer = "trailer\n<</Size $high /Root $root>>\nstartxref\n$xref_offset\n\%\%EOF";
	#print "trailer = \n$trailer\n";
	
	print $clean_pdf $trailer;
	
	# write the xref tables
	close($clean_pdf);

}


# This function clean all javascript and suspiscious embedded files in the pdf.
sub Rewrite_clean_2{


	my ($filename, $version, $pdfObjects, @trailers) = @_;
	#my %pdfObjects = %{$pdfObjs_ref};
	#$version = "OK";
	$filename ="clean.pdf";
	my $clean_pdf;

	# Create the clean file
	open ($clean_pdf, ">$filename" ) or die "Rewrite_clean :: failed to open file :: filename\n";

	# write header
	print $clean_pdf "$version\n";
	print $clean_pdf "%äüöß\n"; # add binary data for PDF interpretation
	
	
	&RemoveModifyDangerousObjects($pdfObjects);
	
	
	my @objs = values(%{$pdfObjects});
		
	
	foreach(@objs){
	
		if(!($_->{"content"} =~ /\d+\s\d\sobj/) ){
			next;
		}
		
		if(exists($_->{offset}) ){
			seek($clean_pdf,$_->{offset},0);
		}else{
			seek($clean_pdf,0,2);
		}

		#print "writing object $_->{ref} at ".tell($clean_pdf)."\n";
		
		#print $clean_pdf $_->{"ref"};
		print $clean_pdf $_->{"content"};
		print $clean_pdf "\n";
		
		# fix "zero" bug by remplacing with carriage
		# read the next byte an replace if it's zero
		#my $b;
		#print "b = $b\n";
		#read $clean_pdf, $b, 1;
		#if( $b == 00  ){
		#}
		#print $clean_pdf "endobj\n\n";
	}
	
	
	# write the trailers
	seek($clean_pdf,0,2);
	foreach(@trailers){
		#print "trailer __ $_\n";
		print $clean_pdf $_;	
	}
	
	
	#print $main::dblezo;
	
	# write the xref tables
	close($clean_pdf);

}

# This function clean all javascript and suspiscious embedded files in the pdf.
sub Rewrite_clean_1{


	#my $filename = shift;
	my ($filename, $version, $pdfObjects, @trailers) = @_;
	#my %pdfObjects = %{$pdfObjs_ref};
	#$version = "OK";
	$filename ="clean.pdf";
		
	my $clean_pdf;

	

	# Create the clean file
	open ($clean_pdf, ">$filename" ) or die "Rewrite_clean :: failed to open file :: filename\n";

	# write header
	print $clean_pdf "$version\n";
	print $clean_pdf "%äüöß\n"; # add binary data for PDF interpretation
	
	my @objs = values(%{$pdfObjects});
	
	foreach(@objs){
	
		if(exists($_->{offset})){
			seek($clean_pdf,$_->{offset},0);
		}else{
			seek($clean_pdf,0,2);
		}
		
		print "Writing object $_->{ref} at ".tell($clean_pdf)."\n" unless $DEBUG eq "no";
		
		#print $clean_pdf $_->{"ref"};
		print $clean_pdf $_->{"content"};
		print $clean_pdf "endobj\n\n";
	}
	
	#print "Position in file :: ".tell($clean_pdf)."\n";
	#foreach(sort(@objs)){
	#	print "$_\n";	
	#}
	
	# write the trailers
	seek($clean_pdf,0,2);
	foreach(@trailers){
		#print "trailer __ $_\n";
		print $clean_pdf $_;	
	}
	
	# write the xref tables
	close($clean_pdf);

}


1;;

__END__
