#!/usr/bin/perl -w 

use strict;
#use Compress::Raw::Zlib;


#use IO::Uncompress::Inflate qw(inflate $InflateError);
use Fcntl;
use Fcntl qw(:DEFAULT :flock);
use bytes;


# libraries
use lib::utils::Filters;	# Filters
use lib::analysis::DocumentStruct;
use lib::analysis::ObjectAnalysis;
use lib::analysis::CVEs;


# VARIABLES


my @to_analyze; # Elements to analyze
#my @obj;
#my @pdf_objects; # list of pdf Objects #TODO change the list in hash table with each key represent the reference of the value;
my @trailers;	# List of trailers

my %pdfObjects;

# Error codes
my $MAGIC_OK = 0;
my $BAD_MAGIC = -2;
my $BAD_XREF_OFFSET = -4;
my $BAD_OBJ_OFFSET = -3;

my $pdf_version; # PDF version

my %TESTS_CAT_1; # Document structure tests
my %TESTS_CAT_2; # Objects analysis tests
my %TESTS_CAT_3; # CVEs

$TESTS_CAT_2{"Dangerous Pattern High"} = 0;
$TESTS_CAT_2{"Dangerous Pattern Medium"} = 0;
$TESTS_CAT_2{"Dangerous Pattern Low"} = 0;


my $DEBUG = "no"; # Print debug infos
my $ENCRYPTED = "no";
my $SUSPICIOUS = 0; # suspicious coef

# object structure
#object -> %information ($key => $value);
#	-> content ();

# object is a hash table with two keys "information" and "content"
	# The information value is a reference to a hash table with all informations
	# The content value is the stream content.


# Test3 files	:: global object parameters or dictionaries linked to script (Ex: Author) sctring.fromCharcode
# 779cb6dc0055bdf63cbb2c9f9f3a95cc
# 80ac1a1642e89bb8738d85788603f5c3
# 968c011f5b2df0784cd1d63a1027ac63
# a447cc071a19af8041448ab944b4d555
# a73f8969ca1abbb8a8ab8cd4d0f8a1a5
# c45451017c90fd836df3aaf33e159cfa /!\ producer param in catalogue
# c739e57001b11dae100d0f490b485817
# d35f7666340804c99768149e13ff62cd
# ddcd296dd8557d3808a5ff87c726b4a9
# edab6ed2809f739b67667e8fed689992
# fed02d9a12222c157c650ee5e8b95edc

# Test4 files	:: Bad pdf struct detection
# 779cb6dc0055bdf63cbb2c9f9f3a95cc

# TODO See JavaScript for Acrobat API reference
#  app.trustedFunction

#6x<9c><8d>XÛnÜ6x<9c><8d>nÜ6x<9c><8d>XÛnÜ6

# TODO Analyse recursive d'un pdf embedded

# TODO Presence de JavaScript sans formulaire
# TODO Look for embedded files types
############################################################





# The basic analysis consists to parse the content of object and detect all potential dangerous patterns.
# Returns "none" - "high" - "medium" - or "low"
sub DangerousKeywordsResearch{

	# 
	#$TESTS_CAT_2{"Dangerous Pattern High"} ;
	#$TESTS_CAT_2{"Dangerous Pattern Medium"};
	#$TESTS_CAT_2{"Dangerous Pattern Low"};

	my ($obj_ref,$content) = @_;
	
	if(!$content){
		#print "Error :: DangerousKeywordsResearch :: empty content\n";
		return "err";
	}
	
	# Put pattern in categories
	
	#my @objs = keys(%pdfObjects);
	
	#foreach (@objs){
	
		# Trigger Launch actions (HIGH)
		#if($_->{"action"} eq "/Launch"){
		#	$TESTS_CAT_2{"Dangerous Pattern High"} ++;
		#	print "Warning :: :: /Launch action detected\n";
		#	return ;
		#}
		
		# keywords (HIGH) :: HeapSpray - heap - spray - hack - shellcode - shell - Execute - exe - exploit - pointers - memory - exportDataObject -app.LaunchURL -byteToChar
		if( $content =~ /(HeapSpray|heap|spray|hack|shellcode|shell|Execute|exe|exploit|pointers|memory|app\.LaunchURL|byteToChar)/si ){
			$TESTS_CAT_2{"Dangerous Pattern High"} ++;
			print "Dangerous Pattern \(High\) found :: $1 :: in $obj_ref->{ref} \n";
			return "high";
		}

		
		# Javascript keywords (MEDIUM) :: substring - toSring - split - eval - String.replace - unescape - exportDataObject - StringfromChar
		if( $content =~ /(toString|substring|split|eval|addToolButton|String\.replace|unescape|exportDataObject|StringfromChar)/si ){
			$TESTS_CAT_2{"Dangerous Pattern Medium"} ++;
			print "Dangerous Pattern \(Medium\) found :: $1 :: in $obj_ref->{ref} \n";
			return "medium";
		}
		
		
		
			
	#}

	# javascript keywords :: 
	# 
	# 
	# NOP detection "90"
	# 
	# %u... like   %u4141%u4141%u63a5%u4a80%u0000
	
	# Look for JavaScript in XFA block Ex: <script name="ADBE::FileAttachment" contentType="application/x-javascript" ></script>
	
	return "none";
}






# This function detects hexa obfuscation in pdf objects fields and decode it.
sub Hexa_Obfuscation_decode{

	my $obj_ref = shift;
	my $dico;
	my ($pre,$post);
	my $case =-1;
	
	if( ! exists($obj_ref->{"content"})){
		return;
	}
	
	
	# TODO : case where it's only #20 replacement
	# Get Dictionary
	if($obj_ref->{"content"} =~ /<<(.*)>>\s*stream/s){
	
		$dico = $1;
		#$pre = $`;
		#$post = $';
		#$case =1;
		
		my $tmp = $dico;
		$dico =~ s/#([0-9A-Fa-f]{2})/pack("C", hex($1))/ge;
		#print "DEBUG1 :: $tmp ::=> $dico \n";
		# If the dico has been modified
		if( $tmp ne $dico){

			#my $dico_d = $obj_ref->{"content"};	
			# replace dico
			#$content_d =~ s/\Q$tmp\E/\Q$dico\E/s;
			
			$obj_ref->{"dico_d"} = $dico;
			#print "DEBUG 3::\n";
						
		}
		
		
		
	}elsif($obj_ref->{"content"} =~ /<<(.*)>>/s){
	
		#print "DEBUG2 :: $1\n";
		$dico = $1;
		#$pre = $`;
		#$post = $';
		#$case = 2;
		
		my $tmp = $dico;
		$dico =~ s/#([0-9A-Fa-f]{2})/pack("C", hex($1))/ge;
		#print "DEBUG2 :: $tmp ::=> $dico \n";
		# If the dico has been modified
		if( $tmp ne $dico){
		
			#my $dico_d = $obj_ref->{"content"};
			# replace dico
			#$content_d =~ s/(\Q$tmp\E)/(\Q$dico\E)/s;
			
			$obj_ref->{"dico_d"} = $dico;
			#print "DEBUG 3::\n";	
		}
	}
	

}


# This function return the number of active " potentially dangerous" contents in the pdf
# This function return the reference of object to analyse.
# TODO : 
sub Active_Contents{
	
	my @to_analyse;
	my $active_content =0;
	
	my @objs = values(%pdfObjects);
	
	foreach(@objs){
	
		
		while ( (my $key, my $value) = each %{$_}){
		
			if($key =~ /js|javascript/){
				print "Warning :: Active_Contents :: Found active content :: $key in  $_->{ref}\n" unless $DEBUG eq "no";
				$active_content ++;
			}
			
			if($key eq "type" and ( $value eq "/EmbeddedFile" )){# or $value eq "/Names")){
				print "Warning :: Found active content :: $value in $_->{ref}\n" unless $DEBUG eq "no";
				$active_content ++;
			}	
		}
		
		
		# XFA 
		if(exists($_->{"xfa"}) ){
			
			# an array of object
			my @xfas = $_->{"xfa"} =~ /(\d+\s\d\sR)/sg;
			
			#print @xfas;
			
			foreach (@xfas){
			
				my $xfa = $_;
				$xfa =~ s/R/obj/;
				print "found XFA obj :: $xfa\n";
				
				if(exists($pdfObjects{$xfa})){
				
					#print "found XFA obj :: $xfa\n";
					if(exists($pdfObjects{$xfa}->{"stream_d"}) && length($pdfObjects{$xfa}->{"stream_d"})>0 ){
						
						# Search javascript content
						# <script contentTyp='application'contentType='application/x-javascript'>
						if($pdfObjects{$xfa}->{"stream"} =~ /javascript/si){
							print "found javaScript in XFA : $xfa\n";
							$active_content ++;
						}
						
					}elsif(exists($pdfObjects{$xfa}->{"stream"}) && length($pdfObjects{$xfa}->{"stream"})>0){
					
						if($pdfObjects{$xfa}->{"stream"} =~ /javascript/si){
							print "found javaScript in XFA :: $xfa\n";
							$active_content ++;
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






# This function extract other object inside object stream
# TODO fix bug :: objects not in the rigth order. ()Ex: cerfa_13753-02.pdf :: 16 0 obj)
sub Extract_From_Object_stream{

	
	#print "\n\n ::EXTRACT OBJ FROM OBJECT STREAM\n";
	
	
	my @objs = values(%pdfObjects);
	
	#if(exists($pdfObjects{$obj_ref}) && $pdfObjects{$obj_ref}->{"type"} eq "/XRef"  ){
	
	# Look for object stream
	for(@objs){
	
		if(exists($_->{"type"}) && $_->{"type"} =~ /ObjStm/ && exists($_->{"stream_d"}) && length($_->{"stream_d"}) > 0 ){
		
			print "Found object stream :: $_->{ref} :: $_->{N} :: $_->{first} :: == $_->{stream_d} \n" unless $DEBUG eq "no";
			print "Found object stream :: $_->{ref} ::\n == $_->{stream_d}";
			#if($_->{"ref"} eq "16 0 obj"){
			#	print $_->{"stream_d"}."\n\n\n\n";
				
			#}

			# Get the list of objects inside
			# 122 0 : 123 543 :

			my $num = $_->{"N"};
			my @obj_inside = $_->{"stream_d"} =~ /(\d+\s\d+)/sig;


			#my @obj_inside_content = $_->{"stream_d"} =~ /(<<[A-Za-z\s\d]+>>)/sig;
			#print "";
			
			if($#obj_inside < 0 ){
				return ;
			}
			
			if($#obj_inside < $num){
				$num = $#obj_inside;
			}
			
			for(my $i=0; $i<$num; $i++){
			
				
				#print $obj_inside[$i]."\n";
				my $obj_num = 0;
				my $obj_off = 0;
				my $obj_off_next = -1;

				if($obj_inside[$i] =~ /(\d+)\s(\d+)/s){
					$obj_num = $1;
					$obj_off = $2;
				}else{
					print "Warning :: obj num and obj offset not found in :: $obj_inside[$i] :: size=$#obj_inside  ::i = $i :: n = $num\n";
				}

				if( $i != $num-1 && $i+1 < $num && ($obj_inside[$i+1] =~ /(\d+)\s(\d+)/s)){
					$obj_off_next = $2;
					#if($_->{"ref"} eq "16 0 obj"){
						#print "DEBUG1 :: \n";
					#}
				}

				my %new_obj;
				my $oref = "$obj_num 0 obj"; # object reference
				$new_obj{"ref"} = $oref;
				#
				# substr EXPR,OFFSET,LENGTH
				my $off= $_->{"first"} + $obj_off;
				if($obj_off_next != -1){
					my $len= $obj_off_next - $obj_off;
					#$new_obj{"content"} = substr ($_->{"stream_d"}, $off, $len) or print "Warning:: substr :: off=$off :: len=$len :: obj_num=$obj_num\n== $_->{stream_d} \n\n\n\n";
					$new_obj{"content"} = substr ($_->{"stream_d"}, $off, $len);
				}else{
					#if($_->{"ref"} eq "20 0 obj"){
					#	print "DEBUG :: $off \n";
					#}
					$new_obj{"content"} = substr ($_->{"stream_d"}, $off);					
				}
				#$new_obj{"content"} = substr ($_->{"stream_d"}, $off, $len );

				print "\nObject content :: $oref :: == ".$new_obj{"content"}."\n" unless $DEBUG eq "no";

				if($new_obj{"content"}){
					if(exists($new_obj{"content"}) && length($new_obj{"content"}) > 0 ){
						&GetObjectInfos(\%new_obj);
					}
				}
				
				
				# Add found object in list
				#if(exists($new_obj{"content"})){
				$pdfObjects{$oref}=\%new_obj;
				#}
				
				
			}

			#print "Number of object inside = $#obj_inside\n";
			#foreach (@obj_inside){
			#	print "$_\n";
			#	pull ,$_
			#}
			
		}
		
	}
}


# This function checks incoherences in the document format (Ex: Empty pages but only js script).
sub Document_struct_detection{

	my ($content,$fh) = @_;
	my $result =0;
	my $ret = 0;
	my $active_content = 0;
	my $empty =0;

	print "\n:::TEST 4 = PDF structure:::\n" unless $DEBUG eq "no";
	
	
	# Get active content (js, embedded files )
	$active_content = &Active_Contents(\%pdfObjects);

	print "DEBUG 1 ::".\%pdfObjects."\n";
	# Check if all pages are empty and there is only js or embedded file
	$empty = &DocumentStruct::Empty_Pages_Document_detection(\%pdfObjects);
	
	# If all pages are empty and there is active content
	if($empty == 0 && $active_content > 0 ){
		print "\t=> Empty PDF document with active content !!" unless $DEBUG eq "no";
		$TESTS_CAT_1{"Empty Doc With Active Content"}="DETECTED";
	}elsif($active_content > 0){
		print "\t=> Potentially dangerous content ($active_content) found in this document !!\n" unless $DEBUG eq "no";
		$TESTS_CAT_1{"Active Content"} = $active_content;
		$TESTS_CAT_1{"Empty Doc With Active Content"}="OK";
	}else{
		$TESTS_CAT_1{"Empty Doc With Active Content"}="OK";
	}
	
	
	# stream with Length = 0

	# TODO Verification des fonctions natives javascripts


	# Check xref table or xref stream object
	if($#trailers >=0){
	
		foreach(@trailers){
			print "trailer xx = $_\n" unless $DEBUG eq "no";
			$result+= &DocumentStruct::Check_xref($_,$fh,\%pdfObjects); # Good result if result > 0
		}
		
		if($result > 0){ # test => OK
			print "XREF OFFSET TEST ==> OK\n" unless $DEBUG eq "no";
			$TESTS_CAT_1{"XRef"} = "OK";
						
		}else{
			print "XREF OFFSET TEST ==> BAD XREF OFFSET\n" unless $DEBUG eq "no";
			$TESTS_CAT_1{"XRef"} = "BAD_XREF_OFFSET";
		}
	}else{
		print "No trailer or startxref section found !!\n" unless $DEBUG eq "no";
		$TESTS_CAT_1{"Trailer"} = "TRAILER_NOT_FOUND";
	}

	# TODO Check pages

}


# This function check weird correlation between objects contents and js script or embedded files.
sub Object_correlation_detection{

	# Check object parameter for weird content Ex: catalogue
	my @objs = values(%pdfObjects);
	foreach (@objs){

		# Check catalogue content
		if(exists($_->{type}) && $_->{type} =~ /Catalog/){
			print "Processing Catalog....\n";
		}

		# Check Info content
		if(exists($_->{type}) && $_->{type} =~ /Info/){
			print "Processing Info....\n";
			while((my $k, my $v)=each(%{$_})){

				if(!($k =~ /content|ref/)){
					print "DEBUG :: $k ==> $v\n";
					Shellcode_Detection($v);
				}
			}
		}

		# TODO Get basic pattern search on Info content and catalogue (string text).


		# TODO Get XFA stream or array		
	}

}


# Performes an deeper analysis of each objects
sub ObjectAnalysis{

	print "\n\n::: Object Analysis:::\n" unless $DEBUG eq "no";
	my @objs = values(%pdfObjects);
	
	my $pattern_rep = 0;
	my $shellcode = 0;
	my $dangerous_pat = 0;
	
	foreach (@objs){
	
	
		if(exists($_->{"action"}) &&$_->{"action"} eq "/Launch"){
			$TESTS_CAT_2{"Dangerous Pattern High"} ++;
			print "Warning :: :: /Launch action detected\n";
		}	
		
		# Analyse Info obj for suspicious strings
		if(exists($_->{"type"}) && $_->{"type"} eq "/Info"){
		
			#if($result > 0){
			#	$TESTS_CAT_2{"Pattern Repetition"} = "DETECTED";
			#}
			
			$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($_->{"content"});
			$shellcode += &ObjectAnalysis::Shellcode_Detection($_->{"content"});
			
			my $res = DangerousKeywordsResearch($_, $_->{"content"});
			if($res ne "none"){
				print "Dangerous keyword \($res\) in Info object $_->{ref}\n";
				$TESTS_CAT_2{"Dangerous Pattern High"} ++;
			}	
		}

		
		# Analyse javascript content
		if(exists($_->{"js_obj"})){
		
			my $js_obj_ref = $_->{"js_obj"};
			$js_obj_ref =~ s/R/obj/;
			
			print "Analysing Object : $js_obj_ref \n" unless $DEBUG eq "no";
			
			if(! exists($pdfObjects{$js_obj_ref}) ){
				print "Object to analyse $js_obj_ref is not defined\n" unless $DEBUG eq "no";
				next;
			}
			
			if(exists($pdfObjects{$js_obj_ref}->{stream_d})){
				$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$js_obj_ref}->{stream_d});
				$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$js_obj_ref}->{stream_d});
				DangerousKeywordsResearch($pdfObjects{$js_obj_ref}, $pdfObjects{$js_obj_ref}->{stream_d});
				
			}elsif(exists($pdfObjects{$js_obj_ref}->{stream})){
				$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$js_obj_ref}->{stream});
				$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$js_obj_ref}->{stream});
				DangerousKeywordsResearch($pdfObjects{$js_obj_ref}, $pdfObjects{$js_obj_ref}->{stream});
			}
				
		}elsif(exists($_->{js})){
			$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($_->{"js"});
			$shellcode += &ObjectAnalysis::Shellcode_Detection($_->{"js"});
			DangerousKeywordsResearch($_, $_->{"js"});
		}
		
		
		
		# Analyse AcroForm, XFA
		if(exists($_->{"acroform"})){
		
			my $acrform_ref = $_->{"acroform"};
			$acrform_ref =~ s/R/obj/;
			
			print "Analysing Object : $acrform_ref \n" unless $DEBUG eq "no";
			
			if(! exists($pdfObjects{$acrform_ref}) ){
				print "Object to analyse $acrform_ref is not defined\n" unless $DEBUG eq "no";
				next;
			}
			
			# Get XFA 
			if(exists($pdfObjects{$acrform_ref}->{xfa})){
			
				#my $xfa = $pdfObjects{$acrform_ref}->{xfa} ;
				
				# If it's an array
				
				# an array of object
				my @xfas = $pdfObjects{$acrform_ref}->{xfa} =~ /(\d+\s\d\sR)/sg;
			
				foreach (@xfas){
			
					my $xfa = $_;
					$xfa =~ s/R/obj/;
					print "Treating XFA object :: $xfa\n" unless $DEBUG eq "yes";
				
					if(exists($pdfObjects{$xfa})){
				
						#print "found XFA obj :: $xfa\n";
						if(exists($pdfObjects{$xfa}->{"stream_d"}) && length($pdfObjects{$xfa}->{"stream_d"})>0 ){
							$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$xfa}->{stream_d});
							$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$xfa}->{stream_d});
							DangerousKeywordsResearch($pdfObjects{$xfa},$pdfObjects{$xfa}->{stream_d});	
										
						}elsif(exists($pdfObjects{$xfa}->{"stream"}) && length($pdfObjects{$xfa}->{"stream"})>0){
							$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$xfa}->{stream});
							$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$xfa}->{stream});
							DangerousKeywordsResearch($pdfObjects{$xfa},$pdfObjects{$xfa}->{stream});
						}
					}
				
				}
				
			}
	
		}
		
		# TODO embedded files
				
	}
	
	if($pattern_rep > 0){
		$TESTS_CAT_2{"Pattern Repetition"} = $pattern_rep;
	}
	
	if($shellcode > 0){
		$TESTS_CAT_2{"Shellcode"} = $shellcode;
	}
	

}


# Get filter applied to a stream
sub GetStreamFilters{

	my $obj_content = shift;
	my @filter_list;

	# If there is only one filter - Ex: /Filter /Flatecode
	if( $obj_content =~ /\/Filter\s*\/([A-Za-z\d]*)/ig ){
		push @filter_list, $1;
	}elsif($obj_content =~ /\/Filter\s*\[\s*([A-Za-z\/\s\d]*)\s*\]/ig){ # For several filters - Ex

		my $tmp = $1;
		@filter_list= $tmp =~ /\/([A-Za-z\d]*\s*)/ig;	
	}

	return @filter_list;

}


# This function decode Xref Stream according to Predictor
sub DecodeXRefStream{

	my ($obj_ref,$stream) = @_;
	
	my $tmp;
		
	if(length($stream) <= 10 ){
		return;
	}
		
	my @xref_d;
	
	print "stream :: $stream :: len = ".length($stream)."\n" unless $DEBUG eq "no";
	
	# Remove the last 10 characters of the string
	$tmp = $stream;
	
	# calc the number of columns (number of byte in each row) W [1 2 1] => 1+2+1 = 4 ; 4+1 = 5
	my $num = 0;
	my ($byte1,$byte2,$byte3) = (0,0,0); # Size in byte of each field
	
	if(exists($obj_ref->{"w"})){
		#print "$obj_ref->{w} :::\n";
		#if($obj_ref->{w} =~ /\[(\d+)\s(\d+)\s(\d+)\]/sg){
		if($obj_ref->{w} =~ /(\d)\s(\d)\s(\d)/){
			$byte1 = $1;
			$byte2 = $2;
			$byte3 = $3;
			$num = $1 + $2 + $3 + 1;
		}
	}
	
	
	#print "$byte1 :: $byte2 :: $byte3 :: $num\n";
	
	# Split the string into rows (Remove the last 10 characters of the string)
	my @rows;
	for (my $i =0 ; $i < length($stream)-10 ; $i+=$num ){
		my $str = substr ($tmp,$i,$num);
		push @rows, $str;
	}
	
	# print rows
	# The first byte on the row will be the predictor type
	foreach(@rows){
		#print "-> $_\n";
		# Strip the first byte (predictor type) of each row
		$_ = substr($_,1);
		#print "=> $_\n";
	}
	

	# Initialize prev row
	my @prev;
	for(my $i =0 ; $i < $num-1 ; $i++){
		#$prev[$i] = pack("C",0);
		$prev[$i] = 0;
		#print "$i :: $prev[$i]\n";
	}
	
	
	my @row2 ;
	#print "size = $#rows\n";
	
	for (my $i =0 ; $i <= $#rows ; $i++ ){
	
		# convert the row byte by byte
		@row2 = split('',$rows[$i]);
		
		for (my $j=0 ; $j <= $#row2 ; $j++ ){
		
			# Convert the byte from binary to int and add it to the same byte in the previous row (prev)

			# Convert byte from binary to int					
			my $conv_row = unpack ("C",$row2[$j]);
			my $conv_prev  = $prev[$j];
			
			my $sum = $conv_row + $conv_prev;
			#print "sum = $row2[$j]+$prev[$j] :: $conv_row + $conv_prev = $sum\n";
			
			# convert back integer to bytes
			#$row2[$j] = chr ($sum);
			#$row2[$j] = pack("I",$sum);
			$row2[$j] = $sum;
			#print "saved row = $row2[$j] ::$sum\n";
			#$row2[$j] = pack ("i",$sum);
		}
		
		# split in like described in W Ex [1 2 1]
		
		# convert int to bytes and then convert back to int
		my $r1;
		for(my $k= 0; $k < $byte1 ; $k++){
			$r1 .= $row2[$k];
		}
		$r1 = pack("C$byte1","$r1"); #print "r1 = $r1 \n";
		#$r1 = pack("C","$row2[0]"); print "r1 = $r1 \n";
		$r1 = unpack("C$byte1","$r1"); #print "r1 = $r1 \n";
		
		
		my $r2;
		for(my $k= 0; $k < $byte2 ; $k++){
			$r2 .= $row2[$k+$byte1];
		}
		$r2 = pack("C$byte2","$r2"); #print "r2 = $r2 \n";
		#$r2 = pack("C2","$row2[1]$row2[2]"); print "r2 = $r2 \n";
		$r2 = unpack("C$byte2","$r2"); #print "r2 = $r2 \n";
		
		
		
		my $r3;				
		for(my $k= 0; $k < $byte3 ; $k++){
			$r3 .= $row2[$k+$byte1+$byte2];
		}
		#$r3 = pack("C$byte3",$r3); #print "r3 = $r3 \n";
		$r3 = pack("C$byte3",$r3); #print "r3 = $r3 \n";
		#$r3 = pack("C","$row2[3]"); print "r3 = $r3 \n";
		$r3 = unpack("C$byte3","$r3"); #print "r3 = $r3 \n";
		
		my $res_row = "r1 r2 r3";
		print "Debug :: xref row = ".$r1."-".$r2."-".$r3."\n";
		push (@xref_d, $res_row);
		
		@prev = @row2;
				
	}

	# Store Decoded cross reference table
	$obj_ref->{"xref_d"} = \@xref_d;
	
	
}


# Decode Object Stream using /Filters informations
sub DecodeObjStream{

	my $obj_ref = shift;
	my @filters;
	my $stream = $obj_ref->{"stream"};

	# Get the filters list
	if(exists($obj_ref->{"content_d"})){
		@filters = &GetStreamFilters($obj_ref->{"content_d"});
	}else{
		@filters = &GetStreamFilters($obj_ref->{"content"});
	}

	
	if($#filters > -1){

		foreach(@filters){

			$obj_ref->{"filters"}.= "/$_ ";

			if(/FlateDecode/i ){
				$stream= &Filters::FlateDecode($stream);
			}elsif(/ASCIIHexDecode/i ){
				$stream = &Filters::AsciiHexDecode($stream);
			}elsif(/ASCII85Decode/i){
				$stream = &Filters::ASCII85Decode($stream);
			}elsif(/LZWDecode/i){
				$stream = &Filters::LZWDecode($stream,$obj_ref);
			}elsif(/RunLengthDecode/i){
				#TODO $stream_tmp = RunLengthDecode($stream_tmp);
			}elsif(/CCITTFaxDecode/i){
				#TODO $stream = CCITTFaxDecode($stream);
			}elsif(/JBIG2Decode/i){
				#TODO $stream_tmp = JBIG2Decode($stream_tmp);
			}elsif(/DCTDecode/i){
				#TODO $stream_tmp = DCTDecode($stream_tmp);
			}elsif(/PXDecode/i){
				#TODO $stream_tmp = PXDecode($stream_tmp);
			}elsif(/Crypt/i){
				#TODO $stream_tmp = Crypt($stream_tmp);
			}else{
				print "Filter $_ not defined\n";
			}
		
		}

	}
	

	# Xref decode with predictor
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/XRef"){
		&DecodeXRefStream($obj_ref,$stream);
	}
	
	
	
	$obj_ref->{"stream_d"} = $stream;


	return 0;
	
}



# Get object information from dictionary
sub GetObjectInfos{

	# Get parameters
	#my $obj_content = shift;
	my $obj_ref= shift;
	my $obj_content ="";
	my ($ref, $type, $len, $action, $js, $ef);
	my $dico;

	#print "objct ref =".$obj_ref;
	#print "Object content = $obj_content\n";
	
	if(! exists($obj_ref->{"content"})){
		return;
	}
	
	# Detect hexa obfuscation in object dictionary fields.
	&Hexa_Obfuscation_decode($obj_ref);
	
	$dico = $obj_ref->{"content"};
	
	if(exists($obj_ref->{"dico_d"})){
	
		print "Obfuscated object detected !!\n" unless $DEBUG eq "no";
		
		#$obj_content = $obj_ref->{"content_d"};
		
		$dico = $obj_ref->{"dico_d"};
		
		if(exists($TESTS_CAT_1{"Obfuscated Objects"})){
			$TESTS_CAT_1{"Obfuscated Objects"} ++ ;
		}else{
			$TESTS_CAT_1{"Obfuscated Objects"}=1;
		}

	}	
#	}elsif(exists($obj_ref->{"content"})){
#		$obj_content = $obj_ref->{"content"};
#	}else{
#		return ;
#	}
	
	
	$obj_content = $obj_ref->{"content"};

	# Get object reference
	if( $obj_ref->{"content"} =~ /^(\d+\s\d+\sobj)/si){
		$ref = $1;
		$obj_ref->{"ref"}= $ref;
	}
	
	# Get Dictionary
	if( $obj_ref->{"content"} =~ /<<(.*)>>\s*stream/s){
		$obj_ref->{"dico"}= $1;
	}elsif($obj_ref->{"content"} =~ /<<(.*)>>/s){
		$obj_ref->{"dico"}= $1;
	}
	
	# Get object type:
	if( $dico =~ /\/Type\s*(\/[A-Za-z]*)/si){
		$obj_ref->{"type"}= $1;
	}
	
	# Length
	if( $dico =~ /Length\s*(\d+\s\d\sR)/si){
		$obj_ref->{"length"}= $1;
	}elsif( $dico =~ /\/Length\s*(\d+)/si){
		$obj_ref->{"length"}= $1;
	}
	
	# Length1
	if( $dico =~ /\/Length\d\s*(\d+)/si){
		$obj_ref->{"length1"}= $1;
	}

	# Actions
	if( $dico =~ /\/S\s*\/([A-Za-z]*)/g){
		$obj_ref->{"action"}= $1;
	}

	# Dictionary
	#if( $obj_content =~ /<<(.*)>>/sg){
		#$obj_ref->{"dico"}= $1;
	#}
	


	# JavaScript Action
	if( $dico =~ /^\/JavaScript\s*(\(.*\)|\d*\s\d\sR)/sig){
		$obj_ref->{"javascript"}= $1;
		#print "$1";
	}
	
	# JavaScript Action params
	if( $dico =~ /\/JS\s*(\(.*\))/sig){ # javascript string
		$obj_ref->{"js"}= $1;
	}elsif($dico =~ /\/JS\s*(\d+\s\d\sR)/si){ # indirect reference to an object
		$obj_ref->{"js_obj"} = $1;
	}
	
	#if( $obj_content =~ /\/JS\s*(\(.*\)|\d*\s\d\sR)/sig){
	#	$obj_ref->{"js"}= $1;
		#print "$1";
	#}

	# Stream
	if( $obj_ref->{"content"} =~ /stream\s*(.*)\s*endstream/si){
		#print "hey $1\n";
		$obj_ref->{"stream"}= $1;
	}


	# catalog
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} =~ /Catalog/ ){

		# Version (name)
		if($dico =~ /(Version)/sig){
			$obj_ref->{"version"} = $1;
		}

		# Pages (dico) IR => Indirect reference
		if($dico =~ /\/Pages\s*(\d{1,3}\s\d\sR)/sig){
			$obj_ref->{"pages"} = $1;
		}

		# Names (dico)

		# Dests (dico) IR

		# Open Action (array or TODO dico)
		if($dico =~ /(OpenAction\s*\[.*\])/sig){
			$obj_ref->{"openaction"} = $1;
		}

		# AA (dico)

		# AcroForm (dico)
		if($dico =~ /\/AcroForm\s*(\d+\s\d\sR)/sig){
			$obj_ref->{"acroform"} = $1;
		}

		# Metadata (stream) IR
		
		# Lang (text string)
		if($dico =~ /Lang\s*(\(\S*\))/sig){
			$obj_ref->{"lang"} = $1;
		}

	}
	
	# XFA
	if($dico =~ /\/XFA\s*(\d+\s\d\sR)/sig){
		$obj_ref->{"xfa"} = $1;
	}elsif($dico =~ /\/XFA\s*(\[.*])/sig){
		$obj_ref->{"xfa"} = $1;
	}
	

	# FontDescriptor (Check for CVE 2010 2883)
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} =~ /FontDescriptor/ ){

		# Font name
		if($dico =~ /\/FontName\s*(\/[A-Za-z\-+,]*)/sig){
			$obj_ref->{"fontname"} = $1;
		}

		# FontFiles
		if($dico =~ /\/FontFile\d*\s*(\d+\s\d\sR)/si){
			$obj_ref->{"fontfile"} = $1;
		}
	}

	# Object stream
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/ObjStm" ){

		# Number of compressed objects		 
		if($dico =~ /\/N\s(\d+)/si){
			$obj_ref->{"N"} = $1;
		}

	}

	# Object stream
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/ObjStm" ){


		# Offset of the first compressed object (in the decompressed stream)
		#if($obj_content =~ /\/First\s(\d+)/sig){
		#print $obj_ref->{content}; 
		if($dico =~ /\/First\s*(\d+)/si){
			$obj_ref->{"first"} = $1;
		}

	}


	# Pages tree
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/Pages" ){

		# Immediate children of the node (array of indirect references)
		if($dico =~ /\/Kids\s*(\[[\d+\s\d\sR\s*]*\])/si){
			$obj_ref->{"kids"} = $1;
		}
		
		# Count : the number of page objects descendants of this node (integer)
		if($dico =~ /\/Count\s*(\d+)/si){
			$obj_ref->{"count"} = $1;
		}

	}
	
	# Page
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/Page" ){

		# The parent node (indirect reference)
		if($dico =~ /\/Parent\s*(\d+\s\d\sR)/si){
			$obj_ref->{"parent"} = $1;
		}
		
		# Ressource needed by the page (dico) : could be an empty dictionary
		if($dico =~ /\/Resources\s*(\d+\s\d\sR)/si){
			$obj_ref->{"resources"} = $1;
		}
		
		# Content : A content stream describing the contents of the page (optional stream or array) : /!\If absent, the page is empty./!\
		#if($obj_ref->{"content"} =~ /\/Contents\s*(\[.*\]]|\d+\s\d\sR)/si){
		if($dico =~ /\/Contents\s*(\[.*\]|\d+\s\d\sR)/si){
			$obj_ref->{"pagecontent"} = $1;
		}
		
		# Thumb
		# TODO ... 
		
		# Annotations (array)
		if($dico =~ /\/Annots\s*(\[[\d+\s\d\sR\s*]*\])/si){
			$obj_ref->{"annots"} = $1;
		}
		
		# Additional-actions : action to be performed when the page is opened or closed
		if($dico =~ /\/AA\s*(\d+\s\d\sR)/si){
			$obj_ref->{"addactions"} = $1;
		}
		
		# Metadata
		if($dico =~ /\/Metadata\s*(\d+\s\d\sR)/si){
			$obj_ref->{"metadata"} = $1;
		}
		
		# ...

	}
	
	# XRef stream
	if(exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/XRef" ){

		# The number one greater than the highest objectnumber used in this section.
		# Equivalent to the Size entry in the trailer dictionary
		if($dico =~ /\/Size\s*(\d+)/si){
			$obj_ref->{"size"} = $1;
		}
		
		# Index: An array containing a pair of integers for each subsection in this section.
		if($dico =~ /\/Index\s*(\[[\d\s]*\])/si){
			$obj_ref->{"index"} = $1;
		}
		
		# W: An array containing a pair of integers for each subsection in this section.
		if($dico =~ /\/W\s*(\[\d\s\d\s\d\])/si){
			$obj_ref->{"w"} = $1;
		}
		
		# Prev: An array containing a pair of integers for each subsection in this section.
		if($dico =~ /\/Prev\s*(\d*)/si){
			$obj_ref->{"prev"} = $1;
		}


	}


}



# This function print all object in the list
sub PrintObjList{

	print "\n\n::: Objects LIST :::\n\n";
		
	my @objs = values(%pdfObjects);
	
	foreach (@objs){
		print "\n\n::OBJ:: $_\n";

		# Reference;
		print "Reference = ".$_->{"ref"}."\n";

		while ((my $key, my $value) = each(%{$_}) ){

			if(!($key =~ /stream|content|dico|ref/i)){
				print "$key = $value\n";
			}
			if(($key =~ /stream_d/i) ){
				#print "$key = $value\n";
			}
		}
	}

}

# This function print all object in the list
sub PrintSingleObject{

	my $obj_ref = shift;
	
	print "\n\n::: Object $obj_ref :::\n\n";
	
	
	if( exists($pdfObjects{$obj_ref})){
	
		while ((my $key, my $value) = each($pdfObjects{$obj_ref}) ){
		
			if(!($key =~ /stream|content|dico|ref/i)){
				print "$key = $value\n";
			}
			
			if(($key =~ /stream/i) ){
				print "$key = $value\n";
			}
		}
	
	}else{
		print "Object $obj_ref not referenced !\n";
	}
		
	

}


# This function get the trailer (startxref) params pointing to a XRef stream object (works only starting from version 1.5) 
sub GetPDFTrailers_from_1_5{

	my $content = shift;
	@trailers = $content =~ /(startxref\s*\d+\s*\%\%EOF)/sg;

	print "trailer == @trailers\n" unless $DEBUG eq "no";

	# Get the offset of the Xref stream object
	
}


# This function get the trailer params (available for pdf version previous thant 1.5)
sub GetPDFTrailers_until_1_4{

	my $content = shift;
	my $info;

	@trailers = $content =~ /(trailer.*\%\%EOF)/sig;
	
	# TODO Decode hexa obfuscated trailer dico
	
	
	#print "trailers = @trailers\n";
	# Do not treat encrypted documents

	# Tag the Info object in the trailer as Type = Info
	if($#trailers >= 0){
		#print "trailer found !!\n";
		foreach(@trailers){

			# Do not treat encrypted documents
			if(/\/Encrypt/){
				print "Warning :: Encrypted PDF document!!\n" unless $DEBUG eq "no";
				$TESTS_CAT_1{"Encryption"} = "yes";
			}

			#if(/\/Info\s*(\d{1,3}\s\d\sR)/sig){
			if(/\/Info\s(\d{1,3}\s\d\sR)/sig){
				$info = $1;
				$info =~ s/R/obj/;
				
				
				# Tag the info object and fill params
				if( exists($pdfObjects{$info}) ){

					print "Info obj found !!\n" unless $DEBUG eq "no";					
					my $info_obj = $pdfObjects{$info};
					$info_obj->{type} = "/Info";

					# Title		(text string)
					if( $info_obj->{"content"} =~ /Title\s*(\(.*\))/g){
					#if( $_->{"content"} =~ /\/(Title)/g){
						$info_obj->{"title"}=$1;
					}
					
					# Author	(text string)
					if( $info_obj->{"content"} =~ /(Author)/){
						$info_obj->{"author"}=$1;
					}
					
					# Subject	(text string)
					if( $info_obj->{"content"} =~ /(Subject)/){
						$info_obj->{"subject"}=$1;
					}
						
					# Keywords	(text string)
					if( $info_obj->{"content"} =~ /(Keyword)/){
						$info_obj->{"keyword"}=$1;
					}

					# Creator	(text string)
					# Producer	(text string)
					if( $info_obj->{"content"} =~ /(Producer)/){
						$info_obj->{"producer"}=$1;
					}

					# CreationDate	(date)
					if( $info_obj->{"content"} =~ /(CreationDate)/){
						$info_obj->{"creationdate"}=$1;
					}

					# ModDate	(date)
					if( $info_obj->{"content"} =~ /(ModDate)/){
						$info_obj->{"Moddate"}=$1;
					}

					# Trapped	(name)
					if( $info_obj->{"content"} =~ /(Trapped)/){
						$info_obj->{"trapped"}=$1;
					}
					
					# Xref stream object offset
					if( $info_obj->{"content"} =~ /\/XRefStm\s*(\d+)/si){
						$info_obj->{"xrefstm"}=$1;
					}

				}
			}
		}
	}
	
	return @trailers;

}



# This function gets all objects in pdf file.
sub GetPDFObjects{

	my $content = shift;
	my @objs;
	my $num_obj= 0;
	
	@objs = $content =~ /(\d+\s+\d+\s+obj.*?)endobj/sig;
	
	$num_obj = $#objs+1;
	print "Number of Objects: $num_obj \n" unless $DEBUG eq "no";
	
	# Object in the file
	foreach (@objs){

		my %obj_tmp;
		$obj_tmp{"content"}=$_;
		
		
		# Get Object basic Informations
		&GetObjectInfos(\%obj_tmp);
		
		if(exists($obj_tmp{"stream"})){
			# Decode Object Stream;
			&DecodeObjStream(\%obj_tmp);
		}
		
		# Add object in the list (key = obj indirect ref; value = object hash ref)
		
		my $obj_ref = $obj_tmp{"ref"};
		
		# Detect object reference collision 
		if(! exists($pdfObjects{$obj_ref})){
			# Put the object in the list
			$pdfObjects{$obj_ref} = \%obj_tmp;
		}else{
			print "ERROR :: Document Structure :: Object collision :: Object $obj_ref defined more than once\n" unless $DEBUG eq "no";
			if(exists($TESTS_CAT_1{"Object Collision"})){
				$TESTS_CAT_1{"Object Collision"} ++;
			}else{
				$TESTS_CAT_1{"Object Collision"} = 1;
			}
			
			# Bug fix in some case: When the previous object is empty
			if( exists($pdfObjects{$obj_ref}->{stream}) && length($pdfObjects{$obj_ref}->{stream}) <= 0 ){	
				$pdfObjects{$obj_ref} = \%obj_tmp;
			}
			
		}
		#$pdfObjects{$obj_ref} = \%obj_tmp;

	}
	
}


# This function calculate the suspicious coeficient (max = 100 => MALWARE)
sub SuspiciousCoef{

	$SUSPICIOUS = 0;
	
	# Tests list
	
	# Encryption - Test Eliminatoire
	if(exists($TESTS_CAT_1{"Encryption"}) && $TESTS_CAT_1{"Encryption"} eq "yes"){
		$SUSPICIOUS = "ENCRYPTED_PDF";
		return $SUSPICIOUS ;
	}
	
	# Empty Doc With Active Content - Test Eliminatoire
	if( $TESTS_CAT_1{"Empty Doc With Active Content"} eq "DETECTED"){
		$SUSPICIOUS = 99;
		return $SUSPICIOUS;
	}
	
	
	
	
	
	
	
	# Combination tests
	if( exists($TESTS_CAT_1{"Object Collision"}) && exists($TESTS_CAT_1{"XRef"}) ){
	
		if($TESTS_CAT_1{"Object Collision"} > 0 && $TESTS_CAT_1{"XRef"} ne "OK"){
			$SUSPICIOUS += 98;
		}else{
		
			if( $TESTS_CAT_1{"Object Collision"} > 0){ # Object Collision
				$SUSPICIOUS += 10;	
			}
			
			if( $TESTS_CAT_1{"XRef"} eq "BAD_XREF_OFFSET"){ # Xref
				$SUSPICIOUS += 30;
			}
		}
	}
	
	
	
	# Trailer
	if(exists($TESTS_CAT_1{"Trailer"}) && $TESTS_CAT_1{"Trailer"} eq "TRAILER_NOT_FOUND"){
		$SUSPICIOUS += 40;
	}
	
	# Obfuscated Objects
	if(exists($TESTS_CAT_1{"Obfuscated Objects"}) &&  $TESTS_CAT_1{"Obfuscated Objects"} eq "TRAILER_NOT_FOUND"){
		$SUSPICIOUS += 40;
	}
	
	
	# Active Content
	if(exists($TESTS_CAT_1{"Active Content"}) &&  $TESTS_CAT_1{"Active Content"} > 0 ){
		$SUSPICIOUS += 40;
	}
	
	
	# Shellcode
	if(exists($TESTS_CAT_2{"Shellcode"}) &&  $TESTS_CAT_2{"Shellcode"} > 0){
		$SUSPICIOUS += 40;
	}
	
	# Pattern Repetition
	if(exists($TESTS_CAT_2{"Pattern Repetition"}) &&  $TESTS_CAT_2{"Pattern Repetition"} > 0 ){
		$SUSPICIOUS += 40;
	}
	
	# DangerousKeywordResearch
	if($TESTS_CAT_2{"Dangerous Pattern High"} > 0){
		$SUSPICIOUS = 90;
		return $SUSPICIOUS;
	}
	
	if($TESTS_CAT_2{"Dangerous Pattern Medium"} > 0){
		$SUSPICIOUS += 40;
	}
	
	if($TESTS_CAT_2{"Dangerous Pattern Low"} > 0){
		$SUSPICIOUS += 20;
	}
	
	
	# CVE_2010_2883
	if(exists($TESTS_CAT_3{"CVE_2010_2883"}) &&  $TESTS_CAT_3{"CVE_2010_2883"} eq "DETECTED" ){
		$SUSPICIOUS += 50;
	}
	
	if(exists($TESTS_CAT_3{"CVE_2010_2883"}) &&  $TESTS_CAT_3{"CVE_2010_2883"} eq "BAD_FONT_FILE_LENGTH" ){
		$SUSPICIOUS += 40;
	}


	
	return $SUSPICIOUS;
	


}


# This function print the reports of the analysis
sub AnalysisReport{

	my $filename = shift;
	print "\n:::::::::::::::::::::::::::\n";
	print "::::: ANALYSIS SUMMARY ::::\n";
	print ":::::::::::::::::::::::::::\n\n";

	print " Filename = $filename\n";
	
	my $exTime = time - $^T;
	print " Execution time = $exTime sec\n";
	
	# Document structure tests results
	print "\n:: Document structure tests ::\n";
	while ((my $key, my $value) = each %TESTS_CAT_1){
		print "\t$key\t => $value\n";
	}
	
	# Others detection tests results
	print "\n:: Others detections tests ::\n";
	while ((my $key, my $value) = each %TESTS_CAT_2){
		print "\t$key\t => $value\n";
	}
	
	# CVEs tests results
	print "\n:: CVEs tests ::\n";
	while ((my $key, my $value) = each %TESTS_CAT_3){
		print "\t$key\t => $value\n";
	}
	
	print "\nSuspicious coefficient :: $SUSPICIOUS\n";

}


sub main(){

	my $filename=shift;
	my $file;
	my $content;

	# Open the document
	open $file, "<$filename" or die "open failed in $filename : $! ";
	binmode $file or die "Error :: $!\n";

	# Get the content of the document
	$content = do { local $/; <$file>};


	# Check the header of the file (must be %PDF-1.x)
	my ($version,$status) = &DocumentStruct::CheckMagicNumber($file);
	
	if($status eq "BAD_MAGIC"){
		die "Error :: Bad Header for a PDF file\n";
	}
	
	$TESTS_CAT_1{"Header"} = "OK";
	print "PDF version ".$version."\n";
	
	# Get all pdf objects content in the document
	&GetPDFObjects($content);

	# Get and decode object stream content
	&Extract_From_Object_stream;
	
	
	# Get the trailer definition accoring to PDF version below 1.5
	&GetPDFTrailers_until_1_4($content); # Get PDF trailer (works for pdf version below 1.5)

	# If no trailer have been found	
	if($#trailers <0 && $version =~ /\%PDF-1\.[5|6|7]/){
		&GetPDFTrailers_from_1_5($content); #  Get PDF trailer (works for pdf version starting from 1.5)
	}


	# Print the objects list
	&PrintObjList unless $DEBUG eq "yes";

	# if the document is not encrypted
	if(! exists ($TESTS_CAT_1{"Encryption"})){
		ObjectAnalysis();
	}	

	# PDF STRUCT TESTS
	&Document_struct_detection($content,$file); # Works only for version below 1.5 with no compatibility with previous version

	
	$status = &CVEs::CVE_2010_2883_Detection(\%pdfObjects);
	if( $status ne "none"){
		$TESTS_CAT_3{"CVE_2010_2883"} = $status;
	}
	
	
	# Print execution time
	my $exTime = time - $^T;
	print "\n Execution time = $exTime sec\n" unless $DEBUG eq "no";


	#PrintSingleObject("15 0 obj");
	#PrintSingleObject("16 0 obj");
	
	&SuspiciousCoef;
	
	&AnalysisReport($filename);

	print "--------------------------------------------------------------\n";
	print "--------------------------------------------------------------\n\n";
		
	close $file;
}


&main($ARGV[0]);

