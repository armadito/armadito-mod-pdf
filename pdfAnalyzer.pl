#!/usr/bin/perl -w

use strict;
#use Compress::Raw::Zlib;


#use IO::Uncompress::Inflate qw(inflate $InflateError);
use Fcntl;
use Fcntl qw(:DEFAULT :flock);
use bytes;


# libraries
use lib::conf::Config;
use lib::utils::StructParsing;
use lib::utils::Filters;	# Filters
use lib::utils::CleanRewriting;
use lib::analysis::DocumentStruct;
use lib::analysis::ObjectAnalysis;
use lib::analysis::CVEs;


# VARIABLES
my @to_analyze; # Elements to analyze
#my @obj;
#my @pdf_objects; # list of pdf Objects #TODO change the list in hash table with each key represent the reference of the value;


my %pdfObjects;

# Error codes
my $MAGIC_OK = 0;
my $BAD_MAGIC = -2;
my $BAD_XREF_OFFSET = -4;
my $BAD_OBJ_OFFSET = -3;

my $pdf_version; # PDF version

our %TESTS_CAT_1; # Document structure tests
my %TESTS_CAT_2; # Objects analysis tests
my %TESTS_CAT_3; # CVEs

$TESTS_CAT_2{"Dangerous Pattern High"} = 0;
$TESTS_CAT_2{"Dangerous Pattern Medium"} = 0;
$TESTS_CAT_2{"Dangerous Pattern Low"} = 0;


my $DEBUG = "no"; # Print debug infos


# object structure

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

# TODO See Legal Content Attestation check (DocMDP signature)

# TODO See FDF format

# TODO See XDP format = Ok

# TODO Only analyze updated trailer. (do not get all trailers because the previous are deprecated)

# TODO Bug fix: Get Type entries in dictionary (several sub-dictionaries)
############################################################



# This function return the number of active " potentially dangerous" contents in the pdf
# This function return the reference of object to analyse.
# TODO : 
sub Active_Contents{
	
	my @to_analyse;
	my $active_content =0;
	
	
	my @objs = values(%pdfObjects);
	
	foreach(@objs){
	
		
		if( exists($_->{"js"}) or exists($_->{"javascript"}) or exists($_->{"js_obj"}) ){
			print "Warning :: Active_Contents :: Found javascript in  $_->{ref} :: \n" unless $DEBUG eq "yes";
			#print "content = $_->{content}\n";
			#$js ++;
			$active_content ++;
		}
		
		
		# TODO Embedded File
		if( exists($_->{"type"}) && $_->{"type"} eq "/EmbeddedFile" ){
			print "Warning :: Active_Contents :: Found EmbeddedFile in $_->{ref}\n" unless $DEBUG eq "yes";
			#$ef ++;
			$active_content ++;
		}
		
		if(exists($_->{"type"}) && $_->{"type"} eq "/Filespec"){
			print "Warning :: Active_Contents :: Found Filespec in $_->{ref}\n";
			
			# Get embedded file stream
			if(exists($_->{"ef"})){
				print "Warning :: Active_Contents :: Found embedded file entry in filespec :: $_->{ef} \n";
				my $ef ;
				if($_->{"ef"} =~ /\/F\s*(\d+\s\d\sR)/s){ # Get embedded file object stream
					$ef = $1;
					$ef =~ s/R/obj/;
					print "Warning :: Active_Contents :: Found embedded file object :: $ef\n";
					$active_content ++;
					
					# TODO check the embedded file type.
				}
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
				print "found XFA obj :: $xfa\n" unless $DEBUG eq "no";
				
				if(exists($pdfObjects{$xfa})){
				
					#print "found XFA obj :: $xfa\n";
					if(exists($pdfObjects{$xfa}->{"stream_d"}) && length($pdfObjects{$xfa}->{"stream_d"})>0 ){
						
						# Search javascript content
						# <script contentTyp='application'contentType='application/x-javascript'>
						if($pdfObjects{$xfa}->{"stream_d"} =~ /javascript/si){
							print "Warning :: Active_Contents :: Found javaScript balises in XFA (stream_d) : $xfa\n";
							$active_content ++;
						}
						
					}elsif(exists($pdfObjects{$xfa}->{"stream"}) && length($pdfObjects{$xfa}->{"stream"})>0){
					
						if($pdfObjects{$xfa}->{"stream"} =~ /javascript/si){
							print "Warning :: Active_Contents :: Found javaScript balises in XFA (stream) : $xfa\n";
							$active_content ++;
						}
					}
				}
				
			}
		}

	}
	
	
	return $active_content;
}


# This function checks incoherences in the document format (Ex: Empty pages but only js script).
sub Document_struct_detection{

	my ($content,$fh,@trailers) = @_;
	my $result =0;
	
	my $active_content = 0;
	my $empty =0;

	print "\n:::TEST 4 = PDF structure:::\n" unless $DEBUG eq "no";
	
	
	# Get active content (js, embedded files )
	$active_content = &Active_Contents(\%pdfObjects);

	
	# Check if all pages are empty and there is only js or embedded file
	$empty = &DocumentStruct::Empty_Pages_Document_detection(\%pdfObjects);
	
	# If all pages are empty and there is active content
	if($empty == 0 && $active_content > 0 ){
		print "\t=> Empty PDF document with active content !!" unless $DEBUG eq "no";
		$TESTS_CAT_1{"Empty PDF With Active Content"}="DETECTED";
	}elsif($empty == 0){
		print "Warning :: Document_struct_detection :: No content in PDF pages !\n";
		$TESTS_CAT_1{"Empty PDF With Active Content"}="EMPTY_PAGES";
	}elsif($active_content > 0){
		print "\t=> Potentially dangerous content ($active_content) found in this document !!\n" unless $DEBUG eq "no";
		$TESTS_CAT_2{"Active Content"} = $active_content;
		$TESTS_CAT_1{"Empty PDF With Active Content"}="OK";
	}else{
		$TESTS_CAT_1{"Empty PDF With Active Content"}="OK";
	}
	
	
	# stream with Length = 0


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
	my $dangerous_pat = "none";
	
	foreach (@objs){
	
	
		if(exists($_->{"action"}) && $_->{"action"} eq "/Launch"){
			$TESTS_CAT_2{"Dangerous Pattern High"} ++;
			print "Warning :: Dangerous Pattern High :: /Launch action detected in $_->{ref}\n";
		}	
		
		# Analyse Info obj for suspicious strings
		if( exists($_->{"type"}) && $_->{"type"} eq "/Info"){
		
						
			#$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($_->{"content"});
			my $res1= &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($_->{"content"});
			$shellcode += &ObjectAnalysis::Shellcode_Detection($_->{"content"});
			
			my $res = &ObjectAnalysis::DangerousKeywordsResearch($_, $_->{"content"});
			if($res ne "none"){
				print "Dangerous keyword \($res\) in Info object $_->{ref}\n";
				$TESTS_CAT_2{"Dangerous Pattern High"} ++;
			}
			if($res1 == -1){
				print "Time exceeded in object in Info object $_->{ref} \n";
				$TESTS_CAT_2{"Time exceeded"} ++;	
			}else{
				$pattern_rep += $res1;
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
				#$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$js_obj_ref}->{stream_d});
				my $res = &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$js_obj_ref}->{stream_d});
				$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$js_obj_ref}->{stream_d});
				$dangerous_pat = &ObjectAnalysis::DangerousKeywordsResearch($pdfObjects{$js_obj_ref}, $pdfObjects{$js_obj_ref}->{stream_d});
				
				if($dangerous_pat ne "none"){
					print "Dangerous keyword \($dangerous_pat\) in object $pdfObjects{$js_obj_ref}->{ref}\n";
					$TESTS_CAT_2{"Dangerous Pattern $dangerous_pat"} ++;
				}
				if($res == -1){
					print "Time exceeded in object $pdfObjects{$js_obj_ref}->{ref}\n";
					$TESTS_CAT_2{"Time exceeded"} ++;
					
				}else{
					$pattern_rep += $res;
				}
				

				
			}elsif(exists($pdfObjects{$js_obj_ref}->{stream})){
				#$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$js_obj_ref}->{stream});
				my $res = &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$js_obj_ref}->{stream});
				$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$js_obj_ref}->{stream});
				$dangerous_pat = &ObjectAnalysis::DangerousKeywordsResearch($pdfObjects{$js_obj_ref}, $pdfObjects{$js_obj_ref}->{stream});
				if($dangerous_pat ne "none"){
					print "Dangerous keyword \($dangerous_pat\) in object $pdfObjects{$js_obj_ref}->{ref}\n";
					$TESTS_CAT_2{"Dangerous Pattern $dangerous_pat"} ++;
				}
				if($res == -1){
					print "Time exceeded in object $pdfObjects{$js_obj_ref}->{ref}\n";
					$TESTS_CAT_2{"Time exceeded"} ++;
					
				}else{
					$pattern_rep += $res;
				}
				

			}
				
		}elsif(exists($_->{js})){
			#$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($_->{"js"});
			my $res = &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($_->{"js"});
			$shellcode += &ObjectAnalysis::Shellcode_Detection($_->{"js"});
			$dangerous_pat = &ObjectAnalysis::DangerousKeywordsResearch($_, $_->{"js"});
			if($dangerous_pat ne "none"){
				print "Dangerous keyword \($dangerous_pat\) in  object $_->{ref}\n";
				$TESTS_CAT_2{"Dangerous Pattern $dangerous_pat"} ++;
			}
			if($res == -1){
				print "Time exceeded in object $_->{ref}\n";
				$TESTS_CAT_2{"Time exceeded"} ++;
				
			}else{
				$pattern_rep += $res;
			}


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
			
				
				# the case when it's an array of object
				my @xfas = $pdfObjects{$acrform_ref}->{xfa} =~ /(\d+\s\d\sR)/sg;
			
				foreach (@xfas){
			
					my $xfa = $_;
					$xfa =~ s/R/obj/;
					print "Treating XFA object :: $xfa\n" unless $DEBUG eq "no";
				
					if(exists($pdfObjects{$xfa})){
				
						
						if(exists($pdfObjects{$xfa}->{"stream_d"}) && length($pdfObjects{$xfa}->{"stream_d"})>0 ){
							#$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$xfa}->{stream_d});
							my $res = &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$xfa}->{stream_d});
							$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$xfa}->{stream_d});
							$dangerous_pat = &ObjectAnalysis::DangerousKeywordsResearch($pdfObjects{$xfa},$pdfObjects{$xfa}->{stream_d});
							if($dangerous_pat ne "none"){
								print "Dangerous keyword \($dangerous_pat\) in object $pdfObjects{$xfa}->{ref}\n";
								$TESTS_CAT_2{"Dangerous Pattern $dangerous_pat"} ++;
							}
							
													
							if($res == -1){
								print "Time exceeded in object $pdfObjects{$xfa}->{ref}\n";
								$TESTS_CAT_2{"Time exceeded"} ++;
							}else{
								$pattern_rep += $res;
							}							
										
						}elsif(exists($pdfObjects{$xfa}->{"stream"}) && length($pdfObjects{$xfa}->{"stream"})>0){
						
							#$pattern_rep += &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$xfa}->{stream});
							my $res = &ObjectAnalysis::Unknown_Pattern_Repetition_Detection($pdfObjects{$xfa}->{stream});
							$shellcode += &ObjectAnalysis::Shellcode_Detection($pdfObjects{$xfa}->{stream});
							$dangerous_pat = &ObjectAnalysis::DangerousKeywordsResearch($pdfObjects{$xfa},$pdfObjects{$xfa}->{stream});
							
							if($dangerous_pat ne "none"){
								print "Dangerous keyword \($dangerous_pat\) in object $pdfObjects{$xfa}->{ref}\n";
								$TESTS_CAT_2{"Dangerous Pattern $dangerous_pat"} ++;
							}
							if($res == -1){
								print "Time exceeded in object $pdfObjects{$xfa}->{ref}\n";
								$TESTS_CAT_2{"Time exceeded"} ++;
								#$pattern_rep += 0;
							}else{
								$pattern_rep += $res;
							}

							
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


# This function decode Xref Stream according to Predictor
# TODO rewrite this function and save in the previous row byte values instead of integers




# This function print all object in the list
sub PrintObjList{

	print "\n\n::: Objects LIST :::\n\n";
		
	my @objs = values(%pdfObjects);
	
	foreach (@objs){
		print "\n\n::OBJ:: $_\n";

		# Reference;
		print "Reference = ".$_->{"ref"}."\n";

		while ((my $key, my $value) = each(%{$_}) ){

			if(!($key =~ /stream|content|ref/i)){
				print "$key = $value\n";
			}
			if(($key =~ /stream/i) ){
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
			print "$key = $value\n\n";
		}
	
	}else{
		print "Object $obj_ref not referenced !\n";
	}

}



# This function calculate the suspicious coeficient (max = 100 => MALWARE)
sub SuspiciousCoef{

	my $SUSPICIOUS = 0;
	
	# Tests list
	
	# Encryption - Test Eliminatoire
	if(exists($TESTS_CAT_1{"Encryption"}) && $TESTS_CAT_1{"Encryption"} eq "yes"){
		$SUSPICIOUS = "ENCRYPTED_PDF";
		return $SUSPICIOUS ;
	}
	
	# Empty PDF With Active Content - Test Eliminatoire
	if( $TESTS_CAT_1{"Empty PDF With Active Content"} eq "DETECTED"){
		$SUSPICIOUS = $Config::EMPTY_PAGES_WITH_ACTIVE_CONTENT;
		return $SUSPICIOUS;
	}
	
	if( $TESTS_CAT_1{"Empty PDF With Active Content"} eq "EMPTY_PAGES"){
		$SUSPICIOUS += $Config::EMPTY_PAGES_CONTENT;
	}
	
	# Combination tests
	if( exists($TESTS_CAT_1{"Object Collision"}) && exists($TESTS_CAT_1{"XRef"}) ){
	
		if($TESTS_CAT_1{"Object Collision"} > 0 && $TESTS_CAT_1{"XRef"} ne "OK"){
			$SUSPICIOUS += $Config::OBJECT_COLLISION_PLUS_BAD_XREF;
		}else{
		
			if( $TESTS_CAT_1{"Object Collision"} > 0){ # Object Collision
				$SUSPICIOUS += $Config::OBJECT_COLLISION;	
			}
			
			if( $TESTS_CAT_1{"XRef"} eq "BAD_XREF_OFFSET"){ # Xref
				$SUSPICIOUS += $Config::BAD_XREF_OFFSET;
			}
		}
	}
	
	
	if(exists($TESTS_CAT_1{"Multiple Headers"}) && $TESTS_CAT_1{"Multiple Headers"} > 1){
		$SUSPICIOUS += $Config::MULTIPLE_HEADERS;
	}
	
	
	# Trailer
	if(exists($TESTS_CAT_1{"Trailer"}) && $TESTS_CAT_1{"Trailer"} eq "TRAILER_NOT_FOUND"){
		$SUSPICIOUS += $Config::TRAILER_NOT_FOUND;
	}
	
	if(exists($TESTS_CAT_1{"Trailer"}) && $TESTS_CAT_1{"Trailer"} eq "BAD_TRAILER"){
		$SUSPICIOUS += $Config::BAD_TRAILER;
	}
	
	# Obfuscated Objects
	if(exists($TESTS_CAT_1{"Obfuscated Objects"}) &&  $TESTS_CAT_1{"Obfuscated Objects"} > 0){
		$SUSPICIOUS += $Config::OBFUSCATED_OBJECTS;
	}
	
	
	# Active Content
	if(exists($TESTS_CAT_2{"Active Content"}) &&  $TESTS_CAT_2{"Active Content"} > 0 ){
		$SUSPICIOUS += $Config::ACTIVE_CONTENT;
	}
	
	
	# Shellcode
	if(exists($TESTS_CAT_2{"Shellcode"}) &&  $TESTS_CAT_2{"Shellcode"} > 0){
		$SUSPICIOUS += $Config::SHELLCODE;
	}
	
	# Pattern Repetition
	if(exists($TESTS_CAT_2{"Pattern Repetition"}) &&  $TESTS_CAT_2{"Pattern Repetition"} > 0 ){
		$SUSPICIOUS += $Config::PATTERN_REPETITION;
	}
	
	# DangerousKeywordResearch
	if($TESTS_CAT_2{"Dangerous Pattern High"} > 0){
		$SUSPICIOUS = $Config::DANGEROUS_PATTERN_HIGH;
		return $SUSPICIOUS;
	}
	
	if($TESTS_CAT_2{"Dangerous Pattern Medium"} > 0){
		$SUSPICIOUS += $Config::DANGEROUS_PATTERN_MEDIUM;
	}
	
	if($TESTS_CAT_2{"Dangerous Pattern Low"} > 0){
		$SUSPICIOUS += $Config::DANGEROUS_PATTERN_LOW;
	}
	
	if(exists($TESTS_CAT_2{"Time exceeded"}) && $TESTS_CAT_2{"Time exceeded"} > 0){
		$SUSPICIOUS += $Config::TIME_EXCEEDED;
	}
	

	# CVE_2010_2883
	if(exists($TESTS_CAT_3{"CVE_2010_2883"}) &&  $TESTS_CAT_3{"CVE_2010_2883"} eq "DETECTED" ){
		$SUSPICIOUS += $Config::CVE_2010_2883_DETECTED;
	}
	
	if(exists($TESTS_CAT_3{"CVE_2010_2883"}) &&  $TESTS_CAT_3{"CVE_2010_2883"} eq "BAD_FONT_FILE_LENGTH" ){
		$SUSPICIOUS += $Config::CVE_2010_2883_BAD_FONT_FILE_LENGTH;
	}
	

	return $SUSPICIOUS;
	


}


# This function print the reports of the analysis
sub AnalysisReport{

	my ($filename,$suspicious) = @_;
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
	print "\n:: Objects Analysis tests ::\n";
	while ((my $key, my $value) = each %TESTS_CAT_2){
		print "\t$key\t => $value\n";
	}
	
	# CVEs tests results
	print "\n:: CVEs tests ::\n";
	while ((my $key, my $value) = each %TESTS_CAT_3){
		print "\t$key\t => $value\n";
	}
	
	print "\nSuspicious coefficient :: $suspicious\n\n";

}


sub main(){

	my $filename=shift;
	my $file;
	my $content;
	my @trailers;
	my $suspicious;

	# Open the document
	open $file, "<$filename" or die "open failed in $filename : $! ";
	binmode $file or die "Error :: $!\n";

	# Check the header of the file (must be %PDF-1.x)
	my ($version,$status) = &DocumentStruct::CheckMagicNumber($file);
	print "status = $status\n";
	if($status eq "BAD_MAGIC"){
		print "Error :: Bad Header for a PDF file\n";
		$TESTS_CAT_1{"Header"} = $status;
		&AnalysisReport($filename,-1);
		return $status;
	}
	
	$TESTS_CAT_1{"Header"} = $status;
	print "PDF version ".$version."\n";
	$pdf_version = $version;
	
	# Get the content of the document
	seek ($file, 0 ,0 );
	$content = do { local $/; <$file>};
	
	
	# Get all pdf objects content in the document
	%pdfObjects = &StructParsing::GetPDFObjects($content);
	
	&StructParsing::GetObjOffsets($file,\%pdfObjects,$content);

	# Get and decode object stream content
	&StructParsing::Extract_From_Object_stream(\%pdfObjects);
	
	# Get the trailer definition accoring to PDF version below 1.5
	@trailers = &StructParsing::GetPDFTrailers_until_1_4($content,\%pdfObjects); # Get PDF trailer (works for pdf version below 1.5)

	# If no trailer have been found	
	if($#trailers <0 && $version =~ /\%PDF-1\.[5|6|7]/){
		@trailers = &StructParsing::GetPDFTrailers_from_1_5($content,\%pdfObjects); #  Get PDF trailer (works for pdf version starting from 1.5)
	}

	# Print the objects list
	&PrintObjList unless $DEBUG eq "no";

	#print "ObjectAnalysis...".(time - $^T)."sec\n";
	# if the document is not encrypted
	if(! exists ($TESTS_CAT_1{"Encryption"})){
	
		&ObjectAnalysis();
		
		# PDF STRUCT TESTS
		&Document_struct_detection($content,$file,@trailers); # Works only for version below 1.5 with no compatibility with previous version
		
		$status = &CVEs::CVE_2010_2883_Detection(\%pdfObjects);
		
		if( $status ne "none"){
			$TESTS_CAT_3{"CVE_2010_2883"} = $status;
		}
	}	

	# Print execution time
	my $exTime = time - $^T;
	print "\n Execution time = $exTime sec\n" unless $DEBUG eq "no";


	#PrintSingleObject("30 0 obj");
	#PrintSingleObject("25 0 obj");
	#PrintSingleObject("1 0 obj");
	#PrintSingleObject("10 0 obj");
	#PrintSingleObject("534 0 obj");
	#PrintSingleObject("368 0 obj");
	
	$suspicious = &SuspiciousCoef;
	
	&AnalysisReport($filename,$suspicious);
	
	print "--------------------------------------------------------------\n";
	print "--------------------------------------------------------------\n\n";
	
	if($suspicious ne "ENCRYPTED_PDF"){
		#&CleanRewriting::Rewrite_clean($filename, $pdf_version,\%pdfObjects, @trailers);
	}
	
	
	#&PrintObjList unless $DEBUG eq "yes";


	print "--------------------------------------------------------------\n";
	print "--------------------------------------------------------------\n\n";
		
	close $file;
}


&main($ARGV[0]);

