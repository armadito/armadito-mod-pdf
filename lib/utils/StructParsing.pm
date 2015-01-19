package StructParsing;

use strict;

my $DEBUG = "no";


# This function get the offset in byte of each object
sub GetObjOffsets{

	my ($fh,$pdfObjects,$content) = @_ ;
	
	#my $pdfObjects = shift;
	
	my @objs = values(%{$pdfObjects});
	
	for(@objs){
	
		seek($fh,0,0);
		
		# Search object offset:
		if($content =~ /\s\Q$_->{ref}/){
			#print "Object $_->{ref} is at  $-[0] + 1\n";
			$_->{offset} =  $-[0]+1; 
		}
		#print "";
	
	}

}


# This function decode Xref Stream according to Predictor
# TODO rewrite this function and save in the previous row byte values instead of integers
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
	
	#print "num = $num\n";
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
		
		# save the current as the previous row
		@prev = @row2;
		
		# split in like described in W Ex [1 2 1]
		
		# convert int to bytes and then convert back to int
		my $r1;
		for(my $k= 0; $k < $byte1 ; $k++){
			$r1 .= $row2[$k];
		}
		$r1 = pack("C$byte1",$r1); #print "r1 = $r1 \n"; convert int to bytes
		#$r1 = pack("C","$row2[0]"); print "r1 = $r1 \n";
		$r1 = unpack("C$byte1",$r1); #print "r1 = $r1 \n"; convert back to int
		
		
		my $r2;
		for(my $k= 0; $k < $byte2 ; $k++){
			$r2 .= $row2[$k+$byte1];
		}
		#print "r2 = $r2 \n";
		$r2 = pack("C$byte2",$r2); #print "r2 = $r2 \n";
		#my $j = $byte2*8;
		#$r2 = pack("b$j","$r2"); print "r2 = $r2 \n";
		#$r2 = pack("C3","$row2[1]$row2[2]$row2[3]"); print "r2 = $r2 \n";
		$r2 = unpack("C$byte2",$r2); #print "r2 = $r2 \n";
		
		
		
		my $r3;				
		for(my $k= 0; $k < $byte3 ; $k++){
			$r3 .= $row2[$k+$byte1+$byte2];
		}
		#$r3 = pack("C$byte3",$r3); #print "r3 = $r3 \n";
		$r3 = pack("C$byte3",$r3); #print "r3 = $r3 \n";
		#$r3 = pack("C","$row2[3]"); print "r3 = $r3 \n";
		$r3 = unpack("C$byte3",$r3); #print "r3 = $r3 \n";
		
		my $res_row = "$r1 $r2 $r3";
		#print "Debug :: xref row = ".$r1."-".$r2."-".$r3."\n";
		push (@xref_d, $res_row);
		
		
				
	}

	# Store Decoded cross reference table
	$obj_ref->{"xref_d"} = \@xref_d;
	
	# print xref table
	#print "\n\nXREF Stream\n";
	#foreach(@xref_d){
	#	print "$_\n";
	#}
	
	
}



# Get filter applied to a stream
sub GetStreamFilters{

	my $obj_content = shift;
	my @filter_list;

	# If there is only one filter - Ex: /Filter /Flatecode
	
	if( $obj_content =~ /\/Filter\s*\/([A-Za-z\d]*)/sig){
		push @filter_list, $1;
	}elsif($obj_content =~ /\/Filter\s*\[\s*([A-Za-z\/\s\d]*)\s*\]/ig){ # For several filters - Ex

		my $tmp = $1;
		#@filter_list= $tmp =~ /\/([A-Za-z\d]*\s*)/ig;
		@filter_list= $tmp =~ /\/(\S+)/ig;	
	}

	return @filter_list;

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
			
			print "filter in $obj_ref->{ref} = $_\n" unless $DEBUG eq "no";

			if(/FlateDecode/i or $_ eq "Fl" ){
				$stream= &Filters::FlateDecode($stream);
			}elsif(/ASCIIHexDecode/i or $_ eq "AHx"){
				$stream = &Filters::AsciiHexDecode($stream);
			}elsif(/ASCII85Decode/i or $_ eq "A85"){
				$stream = &Filters::ASCII85Decode($stream);
			}elsif(/LZWDecode/i or $_ eq "LZW"){
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
		#&DecodeXRefStream($obj_ref,$stream);
	}
	
	
	if(exists($obj_ref->{"filters"}) ){
		$obj_ref->{"stream_d"} = $stream;
	}
	


	return 0;
	
}




# This function detects hexa obfuscation in pdf objects fields and decode it.
sub Hexa_Obfuscation_decode{

	my $obj_ref = shift;
	my $dico;
	my ($pre,$post);
	my $case =-1;
	my $status = "none"; 
	
	if( ! exists($obj_ref->{"content"})){
		return "no_content";
	}
	
	
	
	# Get Dictionary
	if($obj_ref->{"content"} =~ /<<(.*)>>\s*stream/s){
	
		$dico = $1;
		#$pre = $`;
		#$post = $';
		#$case =1;
		
		my $tmp = $dico;
		my $sans_space = $dico;
		$dico =~ s/#([0-9A-Fa-f]{2})/pack("C", hex($1))/ge;
		$sans_space =~ s/#(20)/pack("C", hex($1))/ge; # Trigger the case where there is only #20 (space)
		#print "DEBUG1 :: $tmp ::=> $dico \n";
		# If the dico has been modified
		if( $tmp ne $dico){
		
			$obj_ref->{"dico_d"} = $dico;
			print "DEBUG 3 :: tmp = $tmp :: dico = $dico :: tmp2 = $sans_space\n" unless $DEBUG eq "no";
			
			$status = "hex_obfuscation";
			
			if($dico eq $sans_space){
				print "Only #20 = space detection\n" unless $DEBUG eq "no";
				$status = "only_space";
			}
			
				
		}
		
		
		
	}elsif($obj_ref->{"content"} =~ /<<(.*)>>/s){
	
		#print "DEBUG2 :: $1\n";
		$dico = $1;
		#$pre = $`;
		#$post = $';
		#$case = 2;
		
		my $tmp = $dico;
		my $sans_space = $dico;
		$dico =~ s/#([0-9A-Fa-f]{2})/pack("C", hex($1))/ge;
		$sans_space =~ s/#(20)/pack("C", hex($1))/ge; # Trigger the case where there is only #20 (space)
		
		#print "DEBUG2 :: $tmp ::=> $dico \n";
		# If the dico has been modified
		if($tmp ne $dico){
		
			$obj_ref->{"dico_d"} = $dico;
			print "DEBUG 3 :: tmp = $tmp :: dico = $dico :: tmp2 = $sans_space\n" unless $DEBUG eq "no";
			
			$status = "hex_obfuscation";
			
			if($dico eq $sans_space){
				print "Only #20 = space detection\n" unless $DEBUG eq "no";
				$status = "only_space";
			}
			
				
		}
	}
	
	
	return $status;
	

}



# Get object information from dictionary
sub GetObjectInfos{

	# Get parameters
	#my $obj_content = shift;
	#my $obj_ref= shift;
	my $obj_ref = shift;
	
	
	my $obj_content ="";
	my ($ref, $type, $len, $action, $js, $ef);
	my $dico;
	my $status = "none";

	#print "objct ref =".$obj_ref;
	#print "Object content = $obj_content\n";
	
	if(! exists($obj_ref->{"content"})){
		return;
	}
	
	# Detect hexa obfuscation in object dictionary fields.
	$status = &Hexa_Obfuscation_decode($obj_ref);
	
	if($status eq "hex_obfuscation"){
	
		if(exists($main::TESTS_CAT_1->{"Obfuscated Objects"})){
			$main::TESTS_CAT_1->{"Obfuscated Objects"} ++ ;
		}else{
			$main::TESTS_CAT_1->{"Obfuscated Objects"}=1;
		}
		
	}
	
	$dico = $obj_ref->{"content"};
	
	if(exists($obj_ref->{"dico_d"})){
	
		print "Obfuscated object detected !!\n" unless $DEBUG eq "no";
		
		#$obj_content = $obj_ref->{"content_d"};
		
		$dico = $obj_ref->{"dico_d"};
	}	

	
	
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
	if($dico =~ /\/Type\s*(\/Catalog)/si){ # fix bug /Catalog
		$obj_ref->{"type"}="/Catalog";
	}elsif($dico =~ /<<.*>>\/Type\s*(\/[A-Z0-9]+)/si){
	#if($dico =~ /<<\/Type\s*(\/[A-Za-z]*)/si){
		$obj_ref->{"type"}= $1;
	}elsif($dico =~ /\/Type\s*(\/[A-Z]*)<<.*>>/si){
		$obj_ref->{"type"}= $1;	
	}elsif( $dico =~ /\/Type\s*(\/[A-Za-z0-9]*)/si){
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
	if($dico =~ /\/S\s*\/(Launch)/sig){
		$obj_ref->{"action"}= "/Launch";
	}
	elsif( $dico =~ /\/S\s*\/([A-Za-z]*)/g){
		$obj_ref->{"action"}= $1;
	}

	
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
		}#elsif(){
			
		#}
		
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
		
		if($dico =~ /\/Encrypt/){			
			print "Warning :: Found /Encryption param in XRef obj :: Encrypted PDF document!!\n" unless $DEBUG eq "no";
			$main::TESTS_CAT_1{"Encryption"} = "yes";
		}


	}
	
	
	# TODO Info object infos
	if( exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/Info" ){
				
					

		#print "Info obj found !!\n" unless $DEBUG eq "no";					
		
		#$pdfObjects->{$info}->{type} = "/Info";

		# Title		(text string)
		if( $obj_ref->{"content"} =~ /Title\s*(\(.*\))/g){
		#if( $_->{"content"} =~ /\/(Title)/g){
			$obj_ref->{"title"}=$1;
		}
		
		# Author	(text string)
		if( $obj_ref->{"content"} =~ /(Author)/){
			$obj_ref->{"author"}=$1;
		}
		
		# Subject	(text string)
		if( $obj_ref->{"content"} =~ /(Subject)/){
			$obj_ref->{"subject"}=$1;
		}
			
		# Keywords	(text string)
		if( $obj_ref->{"content"} =~ /(Keyword)/){
			$obj_ref->{"keyword"}=$1;
		}

		# Creator	(text string)
		# Producer	(text string)
		if( $obj_ref->{"content"} =~ /(Producer)/){
			$obj_ref->{"producer"}=$1;
		}

		# CreationDate	(date)
		if( $obj_ref->{"content"} =~ /(CreationDate)/){
			$obj_ref->{"creationdate"}=$1;
		}

		# ModDate	(date)
		if( $obj_ref->{"content"} =~ /(ModDate)/){
			$obj_ref->{"Moddate"}=$1;
		}

		# Trapped	(name)
		if( $obj_ref->{"content"} =~ /(Trapped)/){
			$obj_ref->{"trapped"}=$1;
		}
		
		# Xref stream object offset
		if( $obj_ref->{"content"} =~ /\/XRefStm\s*(\d+)/si){
			$obj_ref->{"xrefstm"}=$1;
		}
		
	}
	
	# File Specification
#- FS	= (name) The name of the file system to be used to interprete this file specification.
#- 
#- 
#- DOS	= (byte string) A file specification string representing DOS file name.
#- Mac 	= (byte string) 
#- Unix	= (byte string)
#- ID	= (array) An array of two byte strings constituting a file identifier.
#- V 	= (boolean) A flag indicating whether the file referenced by the file specification is volatile (changes frequently with time).
#- EF	= (dico) A dictionary containing a subset of the keys F, UF, DOS, Mac and Unix, corresponding to the file by those names in the file specification dictionary.
#- RF	= (dico) A dico with the same structure as the EF dictionary, which must also be present. /!\ If this entry is present the type entry is required and the file spec dico must be indirectly referenced.
#- Desc	= (text string) Descriptive text associated with the file specification.
#- CI	= (dico) A collection item dictionary, which is used to create the user interface for portable collections.

	if( exists($obj_ref->{"type"}) && $obj_ref->{"type"} eq "/Filespec" ){
	
		# /EF	(dico) A dictionary containing a subset of the keys F, UF, DOS, Mac and Unix, corresponding to the file by those names in the file specification dictionary.
		if( $obj_ref->{"dico"} =~ /\/EF\s*(<<.*>>)/si){
			$obj_ref->{"ef"}=$1;
		}
		
		# /F	= (string) Required if the DOS, Max and Unix entries are all absent.
		if( $obj_ref->{"dico"} =~ /\/F\s*\(([^\(\)]*)\)/si){
			$obj_ref->{"f"}=$1;
		}
		
		# /UF	= (text string) Unicode string that provides file specification.
		#if( $obj_ref->{"dico"} =~ /\/UF\s*\((.*)\)\/*/si){
		if( $obj_ref->{"dico"} =~ /\/UF\s*\(([^\(\)]*)\)/si){
			$obj_ref->{"uf"}=$1;
		}
	}
	


}




# This function gets all objects in pdf file.
sub GetPDFObjects{

	my ($content, $TESTS_CAT_1)= @_;
	
	my @objs;
	my $num_obj= 0;
	my %pdfObjects;
	
	@objs = $content =~ /(\d+\s+\d+\s+obj.*?endobj)/sig;
	
	#$num_obj = $#objs+1;
	#print "Number of Objects: $num_obj \n" unless $DEBUG eq "no";
	
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
			if(exists($TESTS_CAT_1->{"Object Collision"})){
				$TESTS_CAT_1->{"Object Collision"} ++;
			}else{
				$TESTS_CAT_1->{"Object Collision"} = 1;
			}
			
			# Bug fix in some case: When the previous object is empty
			if( exists($pdfObjects{$obj_ref}->{stream}) && length($pdfObjects{$obj_ref}->{stream}) <= 0 ){	
				$pdfObjects{$obj_ref} = \%obj_tmp;
			}
			
		}
		#$pdfObjects{$obj_ref} = \%obj_tmp;

	}
	
	return %pdfObjects;
	
}


# This function get the trailer params (available for pdf version previous thant 1.5)
sub GetPDFTrailers_until_1_4{

	#my $content = shift;
	my ($content,$pdfObjects) = @_;
	my $info;

	my @trailers = $content =~ /(trailer.*\%\%EOF)/sig;
	
	# TODO Decode hexa obfuscated trailer dico
	
	
	#print "trailers = @trailers\n";

	# Tag the Info object in the trailer as Type = Info
	if($#trailers >= 0){
	
		
		foreach(@trailers){

			#print "== $_\n";
			
			# Check the root entry in the trailer's dictionary
			if($_ =~ /\/Root\s*(\d+\s\d\sR)/s){
				
				my $root = $1;
				$root =~ s/R/obj/;
				
				if(exists($pdfObjects->{$root}->{type}) && $pdfObjects->{$root}->{type} eq "/Catalog"){
					print "Trailer = OK\n" unless $DEBUG eq "no";
				}else{
					print "BAD trailer :: obj $root is not a Catalog\n";
					$main::TESTS_CAT_1{"Trailer"} = "BAD_TRAILER";
				}
				
				
			}else{
				print "BAD_TRAILER\n";
				$main::TESTS_CAT_1{"Trailer"} = "BAD_TRAILER";
			}
			
			
			# Do not treat encrypted documents
			if(/\/Encrypt/){
				print "Warning :: Encrypted PDF document!!\n" unless $DEBUG eq "no";
				$main::TESTS_CAT_1{"Encryption"} = "yes";
			}

			#if(/\/Info\s*(\d{1,3}\s\d\sR)/sig){
			if(/\/Info\s(\d+\s\d\sR)/sig){
				$info = $1;
				$info =~ s/R/obj/;
				
				print "Info obj found !!\n" unless $DEBUG eq "no";
				$pdfObjects->{$info}->{type} = "/Info";
			
				&GetObjectInfos($pdfObjects->{$info});

			}
		}
	}
	
	return @trailers;

}


# This function get the trailer (startxref) params pointing to a XRef stream object (works only starting from version 1.5) 
sub GetPDFTrailers_from_1_5{

	#my $content = shift;
	my ($content, $pdfObjects) = @_;
	
	my @trailers = $content =~ /(startxref\s*\d+\s*\%\%EOF)/sg;

	print "trailer == @trailers\n" unless $DEBUG eq "no";

	# Get the offset of the Xref stream object
	
	# TODO Verify the offset of the XRef stream object
	
	return @trailers;
	
}




# This function extract other object inside object stream
# TODO fix bug :: objects not in the rigth order. ()Ex: cerfa_13753-02.pdf :: 16 0 obj)
sub Extract_From_Object_stream{

	
	#print "\n\n ::EXTRACT OBJ FROM OBJECT STREAM\n";
	
	#my $pdfObjs_ref = shift;
	
	#my %pdfObjects = %{$pdfObjs_ref};
	my $pdfObjects = shift;
	
	my @objs = values(%{$pdfObjects});
	
	#if(exists($pdfObjects{$obj_ref}) && $pdfObjects{$obj_ref}->{"type"} eq "/XRef"  ){
	
	# Look for object stream
	for(@objs){
	
		if(exists($_->{"type"}) && $_->{"type"} =~ /ObjStm/ && exists($_->{"stream_d"}) && length($_->{"stream_d"}) > 0 ){
		
			print "Found object stream :: $_->{ref} :: $_->{N} :: $_->{first} :: == $_->{stream_d} \n" unless $DEBUG eq "no";
			#print "Found object stream :: $_->{ref} ::\n"; #== $_->{stream_d}";

			# Get the list of objects inside
			my $num = $_->{"N"};
			my @obj_inside = $_->{"stream_d"} =~ /(\d+\s\d+)/sig;
			
			my $objStm = $_->{"ref"};


			#my @obj_inside_content = $_->{"stream_d"} =~ /(<<[A-Za-z\s\d]+>>)/sig;
			#print "num = $num\n";
			
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
				}

				my %new_obj;
				my $oref = "$obj_num 0 obj"; # object reference
				$new_obj{"ref"} = $oref;
				
				# Save the parent object stream
				$new_obj{"objStm"}= $objStm;
				
				
				
				#
				# substr EXPR,OFFSET,LENGTH
				my $off= $_->{"first"} + $obj_off;
				
				# Save the offset in object stream
				$new_obj{"objStmOff"} = $off;
				
				if($obj_off_next != -1){
					my $len= $obj_off_next - $obj_off;
					#$new_obj{"content"} = substr ($_->{"stream_d"}, $off, $len) or print "Warning:: substr :: off=$off :: len=$len :: obj_num=$obj_num\n== $_->{stream_d} \n\n\n\n";
					$new_obj{"content"} = substr ($_->{"stream_d"}, $off, $len);
				}else{
					$new_obj{"content"} = substr ($_->{"stream_d"}, $off);					
				}
				#$new_obj{"content"} = substr ($_->{"stream_d"}, $off, $len );

				print "\nObject content :: $oref :: == ".$new_obj{"content"}."\n" unless $DEBUG eq "no";

				if($new_obj{"content"}){
					if(exists($new_obj{"content"}) && length($new_obj{"content"}) > 0 ){
						#print "DBLEZO\n";
						&GetObjectInfos(\%new_obj);
					}
				}
				
				
				# Add found object in list
				#if(exists($new_obj{"content"})){
				$pdfObjects->{$oref}=\%new_obj;
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

1;
__END__
