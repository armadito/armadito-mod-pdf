package CleanRewriting;

use strict;

use lib::utils::Filters;

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
		
			print "Warning :: Active_Contents :: Found javascript in  $_->{ref} :: \n" unless $DEBUG eq "yes";
			
			# case if the javascript is described in another object
			if(exists($_->{"js_obj"})){
			
				# get the object
				my $js_obj = $_->{"js_obj"};
				$js_obj =~ s/R/obj/;
				print "js_obj = $js_obj :: \n";
				
				# erase content
				$pdfObjects->{$js_obj}->{content} = "$js_obj\nendobj";
				
				
			}elsif(exists($_->{"js"})){ # if the js is a string
				
				print "javascript content :: $_->{js} \n";
				
				# If the object is packed in an object stream
				if( exists($_->{"objStm"})  ){
					
					print "javascript object $_->{ref} in Object stream $_->{objStm}\n";
					
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
		}
		
		if( exists($_->{"type"}) && $_->{"type"} eq "/EmbeddedFile" ){
			#print "Warning :: Found EmbeddedFile in $_->{ref}\n" unless $DEBUG eq "yes";
			$ef ++;
			$active_content ++;
		}
		
		# XFA 
#		if(exists($_->{"xfa"}) ){
#			
#			# an array of object
#			my @xfas = $_->{"xfa"} =~ /(\d+\s\d\sR)/sg;
#			
#			#print @xfas;
#			
#			foreach (@xfas){
#			
#				my $xfa = $_;
#				$xfa =~ s/R/obj/;
#				print "found XFA obj :: $xfa\n";
#				
#				if(exists($pdfObjects{$xfa})){
#				
#					#print "found XFA obj :: $xfa\n";
#					if(exists($pdfObjects{$xfa}->{"stream_d"}) && length($pdfObjects{$xfa}->{"stream_d"})>0 ){
#						
#						# Search javascript content
#						# <script contentTyp='application'contentType='application/x-javascript'>
#						if($pdfObjects{$xfa}->{"stream"} =~ /javascript/si){
#							print "found javaScript in XFA : $xfa\n";
#							$active_content ++;
#						}
#						
#					}elsif(exists($pdfObjects{$xfa}->{"stream"}) && length($pdfObjects{$xfa}->{"stream"})>0){
#					
#						if($pdfObjects{$xfa}->{"stream"} =~ /javascript/si){
#							print "found javaScript in XFA :: $xfa\n";
#							$active_content ++;
#						}
#					}
#				}
#				
#			}
#		}
		print "\n\n";
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
	
	
	print "......\n";
	
	my $high = 0;
	my $root = 0;
	
	# Write objects extracted from object stream
	foreach(@objs){
	
		
		if($_->{ref} =~ /(\d+)\s\d\sobj/){
		
			if( $1 > $high){
				$high = $1;
			}
		}
		
		if(exists($_->{type}) && $_->{type} eq "/Catalog"){
			$root = $_->{"ref"};
		}
		
		
		
		# Reach the end of the file
		seek($clean_pdf,0,2);
		

		print "writing object $_->{ref} at ".tell($clean_pdf)."\n";
		
			
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
	
	# TODO Write the XRef
	
	
	# Write the trailer at the end of the file
	$high = $high+1;
	my $trailer = "trailer\n<</Size $high /Root $root>>\nstartxref\n0\n\%\%EOF";
	print "trailer = $trailer\n";
	
	
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
		
		print "writing object $_->{ref} at ".tell($clean_pdf)."\n";
		
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
