package Filters;

use strict;
use Compress::Zlib;
use Math::Trig;

my $DEBUG = "no";

# List of filters:
# FlateDecode
# ASCIIHexDecode
# ASCII85Decode
# LZWDecode
# DCTDecode
# TODO RunLengthDecode
# TODO CCITTFaxDecode
# TODO JBIG2Decode
# TODO PXDecode
# TODO Crypt


# White run Length terminating codes
my %WHITE_RUN_LENGTH_TERMINATING_CODES = (
	'00110101'	=> 0,
	'000111'	=> 1,
	'0111'		=> 2,
	'1000'		=> 3,
	'1011'		=> 4,
	'1100'		=> 5,
	'1110'		=> 6,
	'1111'		=> 7,
	'10011'		=> 8,
	'10100'		=> 9,
	'00111'		=> 10,
	'01000'		=> 11,
	'001000'	=> 12,
	'000011'	=> 13,
	'110100'	=> 14,
	'110101'	=> 15,
	'101010'	=> 16,
	'101011'	=> 17,
	'0100111'	=> 18,
	'0001100'	=> 19,
	'0001000'	=> 20,
	'0010111'	=> 21,
	'0000011'	=> 22,
	'0000100'	=> 23,
	'0101000'	=> 24,
	'0101011'	=> 25,
	'0010011'	=> 26,
	'0100100'	=> 27,
	'0011000'	=> 28,
	'00000010'	=> 29,
	'00000011'	=> 30,
	'00011010'	=> 31,
	'00011011'	=> 32,
	'00010010'	=> 33,
	'00010011'	=> 34,
	'00010100'	=> 35,
	'00010101'	=> 36,
	'00010110'	=> 37,
	'00010111'	=> 38,
	'00101000'	=> 39,
	'00101001'	=> 40,
	'00101010'	=> 41,
	'00101011'	=> 42,
	'00101100'	=> 43,
	'00101101'	=> 44,
	'00000100'	=> 45,
	'00000101'	=> 46,
	'00001010'	=> 47,
	'00001011'	=> 48,
	'01010010'	=> 49,
	'01010011'	=> 50,
	'01010100'	=> 51,
	'01010101'	=> 52,
	'00100100'	=> 53,
	'00100101'	=> 54,
	'01011000'	=> 55,
	'01011001'	=> 56,
	'01011010'	=> 57,
	'01011011'	=> 58,
	'01001010'	=> 59,
	'01001011'	=> 60,
	'00110010'	=> 61,
	'00110011'	=> 62,
	'00110100'	=> 63,
	'000000000001'	=> "EOL"
);

# Black run Length terminating codes
my %BLACK_RUN_LENGTH_TERMINATING_CODES = (
	'0000110111'	=> 0,
	'010'		=> 1,
	'11'		=> 2,
	'10'		=> 3,
	'011'		=> 4,
	'0011'		=> 5,
	'0010'		=> 6,
	'00011'		=> 7,
	'000101'	=> 8,
	'000100'	=> 9,
	'0000100'	=> 10,
	'0000101'	=> 11,
	'0000111'	=> 12,
	'00000100'	=> 13,
	'00000111'	=> 14,
	'000011000'	=> 15,
	'0000010111'	=> 16,
	'0000011000'	=> 17,
	'0000001000'	=> 18,
	'00001100111'	=> 19,
	'00001101000'	=> 20,
	'00001101100'	=> 21,
	'00000110111'	=> 22,
	'00000101000'	=> 23,
	'00000010111'	=> 24,
	'00000011000'	=> 25,
	'000011001010'	=> 26,
	'000011001011'	=> 27,
	'000011001100'	=> 28,
	'000011001101'	=> 29,
	'000001101000'	=> 30,
	'000001101001'	=> 31,
	'000001101010'	=> 32,
	'000001101011'	=> 33,
	'000011010010'	=> 34,
	'000011010011'	=> 35,
	'000011010100'	=> 36,
	'000011010101'	=> 37,
	'000011010110'	=> 38,
	'000011010111'	=> 39,
	'000001101100'	=> 40,
	'000001101101'	=> 41,
	'000011011010'	=> 42,
	'000011011011'	=> 43,
	'000001010100'	=> 44,
	'000001010101'	=> 45,
	'000001010110'	=> 46,
	'000001010111'	=> 47,
	'000001100100'	=> 48,
	'000001100101'	=> 49,
	'000001010010'	=> 50,
	'000001010011'	=> 51,
	'000000100100'	=> 52,
	'000000110111'	=> 53,
	'000000111000'	=> 54,
	'000000100111'	=> 55,
	'000000101000'	=> 56,
	'000001011000'	=> 57,
	'000001011001'	=> 58,
	'000000101011'	=> 59,
	'000000101100'	=> 60,
	'000001011010'	=> 61,
	'000001100110'	=> 62,
	'000001100111'	=> 63,
	'000000000001'	=> "EOL"
);


# White make-up codes
my %WHITE_MAKE_UP_CODES = (
	'11011'		=> 64,
	'10010'		=> 128,
	'010111'	=> 192,
	'0110111'	=> 256,
	'00110110'	=> 320,
	'00110111'	=> 384,
	'01100100'	=> 448,
	'01100101'	=> 512,
	'01101000'	=> 576,
	'01100111'	=> 640,
	'011001100'	=> 704,
	'011001101'	=> 768,
	'011010010'	=> 832,
	'011010011'	=> 896,
	'011010100'	=> 960,
	'011010101'	=> 1024,
	'011010110'	=> 1088,
	'011010111'	=> 1152,
	'011011000'	=> 1216,
	'011011001'	=> 1280,
	'011011010'	=> 1344,
	'011011011'	=> 1408,
	'010011000'	=> 1472,
	'010011001'	=> 1536,
	'010011010'	=> 1600,
	'011000'	=> 1664,
	'010011011'	=> 1728
	
);

# Black make-up codes
my %BLACK_MAKE_UP_CODES = (
	'0000001111'	=> 64,
	'000011001000'	=> 128,
	'000011001001'	=> 192,
	'000001011011'	=> 256,
	'000000110011'	=> 320,
	'000000110100'	=> 384,
	'000000110101'	=> 448,
	'0000001101100'	=> 512,
	'0000001101101'	=> 576,
	'0000001001010'	=> 640,
	'0000001001011'	=> 704,
	'0000001001100'	=> 768,
	'0000001001101'	=> 832,
	'0000001110010'	=> 896,
	'0000001110011'	=> 960,
	'0000001110100'	=> 1024,
	'0000001110101'	=> 1088,
	'0000001110110'	=> 1152,
	'0000001110111'	=> 1216,
	'0000001010010'	=> 1280,
	'0000001010011'	=> 1344,
	'0000001010100'	=> 1408,
	'0000001010101'	=> 1472,
	'0000001011010'	=> 1536,
	'0000001011011'	=> 1600,
	'0000001100100'	=> 1664,
	'0000001100101'	=> 1728
	
);


# Extended make-up codes (Black and White)
my %EXTENDED_MAKE_UP_CODE = (
	'00000001000'	=> 1792,
	'00000001100'	=> 1856,
	'00000001101'	=> 1920,
	'000000010010'	=> 1984,
	'000000010011'	=> 2048,
	'000000010100'	=> 2112,
	'000000010101'	=> 2176,
	'000000010110'	=> 2240,
	'000000010111'	=> 2304,
	'000000011100'	=> 2368,
	'000000011101'	=> 2432,
	'000000011110'	=> 2496,
	'000000011111'	=> 2560
);



# Decompress data encoded using 
sub CCITTFaxDecode{

	my ($stream, $obj_ref) = @_;
	my $out;
	my @results;
	
	#my $columns = 25768;
	#my $rows = 1;

	my $max_bits = 13;
	my $off = 0;
	
	my $color = "white";
	my $res;	
	# TODO Color bit depends of the BlackIs1 entry in dictionary
	my $black = 0;
	my $white = 1;
	
	
	my $bit_stream = unpack("B*",$stream);
	print "CCITTFaxDecode :: Bits Stream  = $bit_stream\n\n\n" unless $DEBUG eq "no";
	
	
	while (length($bit_stream) > 0){
	
		my $buf;
		my $code_len = 0;
		my $run;
		
		
		if($max_bits > length($bit_stream)){
			$max_bits = $bit_stream;
		}
		
		#for ($len= $max_bits; $len>0; $len--){
		for (my $len=1 ; $len<=$max_bits; $len++){
		
		
			# Select 1 to max_bits code
			$buf = substr($bit_stream,$off,$len);
			
			
			# Check in code table
			if($color eq "white"){
			
				if(exists($WHITE_MAKE_UP_CODES{"$buf"})){
				
					$run = $WHITE_MAKE_UP_CODES{"$buf"};
					$code_len = $len;
					#$len = 0;
					$len = $max_bits;
					
					for(my $j = 0; $j < $run; $j++){
						$res .= "$white";
					}
					
					push(@results,$run);
					
					
					
				}elsif(exists($WHITE_RUN_LENGTH_TERMINATING_CODES{"$buf"})){
				
					$run = $WHITE_RUN_LENGTH_TERMINATING_CODES{"$buf"};
					$color = ($color eq "white" ? "black":"white") if $run ne "EOL"; # switch color bit
					$code_len = $len;
					#$len = 0;
					$len = $max_bits;
					
					# Todo remove this line. for tests
					if($run ne "EOL"){
						for(my $j = 0; $j < $run; $j++){
							$res .= "$white";
						}
					}
					
					
					push(@results,$run);
					
				}elsif(exists($EXTENDED_MAKE_UP_CODE{"$buf"})){
				
					$run = $EXTENDED_MAKE_UP_CODE{"$buf"};
					$code_len = $len;
					#$len = 0;
					$len = $max_bits;
					
					for(my $j = 0; $j < $run; $j++){
						$res .= "$white";
					}
					
					push(@results,$run);
				
				}
					
			
			}elsif($color eq "black"){
			
				if(exists($BLACK_MAKE_UP_CODES{"$buf"})){
				
					$run = $BLACK_MAKE_UP_CODES{"$buf"};
					$code_len = $len;
					#$len = 0;
					$len = $max_bits;
					
					for(my $j = 0; $j < $run; $j++){
						$res .= "$black";
					}
					
					push(@results,$run);
					
					
				}elsif(exists($BLACK_RUN_LENGTH_TERMINATING_CODES{"$buf"})){
				
					$run = $BLACK_RUN_LENGTH_TERMINATING_CODES{"$buf"};
					$color = ($color eq "white" ? "black":"white") if $run ne "EOL"; # switch color bit
					$code_len = $len;
					#$len = 0;
					$len = $max_bits;
					
					if($run ne "EOL"){
						for(my $j = 0; $j < $run; $j++){
							$res .= "$black";
						}
					}
					
					push(@results,$run);
					
				}elsif(exists($EXTENDED_MAKE_UP_CODE{"$buf"})){
				
					$run = $EXTENDED_MAKE_UP_CODE{"$buf"};
					$code_len = $len;
					#$len = 0;
					$len = $max_bits;
					
					print "CCITTFaxDecode :: Found Extended make up code !!\n\n" unless $DEBUG eq "no";
					
					for(my $j = 0; $j < $run; $j++){
						$res .= "$black";
					}
					
					push(@results,$run);

				}
			
			}
			
		}
		
		# If the pattern has been found
		if( $code_len > 0 ){
			
			# remove the treated code in the stream
			$bit_stream = substr($bit_stream,$code_len);
			
		}else{
			# move forward (1 bit) in the stream
			$bit_stream = substr($bit_stream,1);
			#$color = ($color eq "white" ? "black":"white");
			print "Warning :: CCITTFaxDecode :: code not found :: length = ".length($bit_stream)."\n" unless $DEBUG eq "no";
		}
	
		
	}
	
	
	# TODO multi-dimensional images decode. (if length(test))
	
	
	print "CCITTFaxDecode :: Run Code results: @results\n\n" unless $DEBUG eq "no";
	print "CCITTFaxDecode :: Bin result :: ".length($res)." = $res\n\n" unless $DEBUG eq "no";
		
	# Translate binary to bytes
	$out = pack("B*",$res);
	print "CCITTFaxDecode :: Out :: ".length($out)." = $out\n\n" unless $DEBUG eq "no";
	

	return $out;

}





# Decompress data encoded using a DCT (Discret cosine transform)
sub DCTDecode{

	my $stream = shift;
	my $out;
	my @xn = split('',$stream);
	my @xk;
	
	#print "stream = $stream\n\n";
	
	my $fh;
	
	# TODO Check if the stream represent a jpeg file. (header=)
	
	open($fh,">idct");
	
	print "Size = $#xn :: ".length($stream)."\n";
	
	#my $int_x0 = ord($xn[0]);
	#print "$xn[0] => $int_x0\n";
	
#	print "$xn[0] => ".ord($xn[0])."\n";
#	print "$xn[1] => ".ord($xn[1])."\n";
#	print "$xn[2] => ".ord($xn[2])."\n";
#	print "$xn[3] => ".ord($xn[3])."\n";
#	print "$xn[4] => ".ord($xn[4])."\n";
#	print "$xn[5] => ".ord($xn[5])."\n\n";
	
	print "$xn[0] => ".unpack("%8C",$xn[0])."\n";
#	print "$xn[1] => ".unpack("%8C",$xn[1])."\n";
#	print "$xn[2] => ".unpack("%8C",$xn[2])."\n";
#	print "$xn[3] => ".unpack("%8C",$xn[3])."\n";
#	print "$xn[4] => ".unpack("%8C",$xn[4])."\n";
#	print "$xn[5] => ".unpack("%8C",$xn[5])."\n";
	
	#print "a => ".unpack("%8C","a")."\n";
	#print "a => ".unpack("%8C","b")."\n";
	
	#my $N = length($stream);
	my $N = $#xn+1;
	my $PI_sur_N = pi/$N;
	print "pi/N => $PI_sur_N\n";
	
	
	
	# for Zero
	
#	my $sum=0;
#	for(my $y=1; $y<$#xn ; $y++){
#	
#		my $int_xn = unpack("%8C",$xn[$y]);
#		#print "$xn[$y] => $int_xn\n";
#	
#		$sum += $int_xn * cos( pi/$N * $y * (0 + 0.5) );
#	}
#	
#	print "sum = $sum\n";
#	my $int_x0 = unpack("%8C",$xn[0]);
#	my $res = 0.5*$int_x0 + $sum;
#	
#	print "res = $res\n";
#	print "$res => ".pack("C",$res)."\n";
	
	# for 1
	
	
#	for(my $i=0; $i<= $#xn; $i++){
			
		
		
		#print "$xn[0] => $int_x0\n";
		
#		my $sum=0;
#		for(my $y=1; $y<$#xn ; $y++){
#		
#			my $int_xn = unpack("%8C",$xn[$y]);
			#print "$xn[$y] => $int_xn\n";
		
#			$sum += $int_xn * cos( pi/$N * $y * ($i + 0.5) );
#		}
		
		#print "$sum = $sum\n";
		
#		my $int_x0 = unpack("%8C",$xn[0]);
#		my $res = 0.5*$int_x0 + $sum;
#		
#		$xk[$i] = pack("C",$res);
#		
#		
#		
#	}
	
#	print @xk;
	
	#$out = join('',@xk);
	$out = $stream;
	
	print $fh $out;
	
	close($fh);

	return $out;
}




# Decode Flatecode streams (returns the decoded stream)
sub FlateDecode{

	my $stream = shift;
	
	my $out;

	#my $len = length($stream); # Size of compressed object
	#print "Stream Length = $len\n";
	#if(length($stream) <= 0){
	#	return;
	#}

	
	# Inflation
	# TODO try WindowBits => -8 ou -15
	#my ($y,$status2) = inflateInit(WindowBits => 15, -BufSize => 1) or die "Error creating inflation stream\n";	# With use Compress::Zlib
	my ($y,$status2) = inflateInit(-BufSize => 1) or die "Error creating inflation stream\n";	# With use Compress::Zlib

	# TODO Chech the header
	#x<9C> => \x78\x9C
	# 78 01 - No compression/low
	# 78 9C - Default Compression
	# 78 DA - Best Compression

	($out, $status2) = $y->inflate($stream) or print "Error inflating the stream\n";
	#print "status = $status2 ::".$y->msg()."\n";

	# 
	#my @arr = split('',$stream);
	#print "arr == @arr\n";
	#foreach(@arr){
	#	$out .= $y->inflate($_) or die "Error inflating the stream\n";		
	#}
	#print "status = $status2 ::".$y->msg()."\n";
	#print "OUT = $out\n";

	return $out;
}

# Decode Flatecode streams (returns the decoded stream)
sub FlateEncode{

	my $stream = shift;
	
	my $out;
	
	my $in="Hello world";


	my $len = length($stream); # Size of compressed object stream
	
	#print "Stream Length = $len\n";
	#chomp($input);

	# All white space characters shall be ignored
	#$input =~ s/\s//sog;

	# Deflation
	#my $x = deflateInit(-Bufsize => 1, -Level => Z_BEST_SPEED ) or die "Error creating deflation stream";
	#my $x = deflateInit(-Bufsize => 1) or die "Error creating deflation stream";
	# Z_NO_COMPRESSION	Z_BEST_SPEED	Z_BEST_COMPRESSION	Z_DEFAULT_COMPRESSION
	my $x = deflateInit(-Bufsize => 1, -Level => 9 ) or die "Error creating deflation stream";
	my ($deflation, $status) = $x->deflate($stream);
	#print "deflation = $deflation\n";
	($out, $status) = $x->flush();
	$deflation .= $out;
	#print "deflation = $deflation\n";
	
	# Inflation
	# TODO try WindowBits => -8 ou -15
	#my ($y,$status2) = inflateInit(WindowBits => 15, -BufSize => 1) or die "Error creating inflation stream\n";	# With use Compress::Zlib
	#my ($y,$status2) = inflateInit(-BufSize => 1) or die "Error creating inflation stream\n";	# With use Compress::Zlib

	# TODO Chech the header
	#x<9C> => \x78\x9C
	# 78 01 - No compression/low
	# 78 9C - Default Compression
	# 78 DA - Best Compression

	return $deflation;
}


#AsciiHexDecode
sub AsciiHexDecode{

	my $stream = shift;
	my $out;
	
	#print "Debug :: AsciiHexDecode :: $stream \n\n";
	
	# All white space characters shall be ignored
	$stream =~ s/\s//sg;

	# "A GREATER-THAN SIGN (3Eh) indicates EOD."
	my $has_eod_marker = 0;
	if (substr($stream, -1, 1) eq '>') {
		$has_eod_marker = 1;
		
		chop $stream;
	}
	
	# Accept only ascii hex-encoded stream
	if($stream=~/[^0-9A-Fa-f]/){
		print "AsciiHexDecode:: Illegal character in stream \n" unless $DEBUG eq "no";
		return ;
	}

	# number of hexadecimal digits, it shall behave as if a 0 (zero)
	# followed the last digit."
	if ($has_eod_marker and length($stream) % 2 == 1) {
		$stream .= '0';
	}

	# "The ASCIIHexDecode filter shall produce one byte of binary data
	# for each pair of ASCII hexadecimal digits."
	$out = $stream;
	$out =~ s/([0-9A-Fa-f]{2})/pack("C", hex($1))/ge;
	
	
	return $out;
}



#ASCII85Decode
sub ASCII85Decode{

	#my ($self, $str, $isend) = @_;
	my $self = {};
	my $str = shift;
	my $isend =0;
	my ($res, $i, $j, @c, $b, $num);
	$num = 0;

	if (exists($self->{'incache'}) && $self->{'incache'} ne "") {
		$str = $self->{'incache'} . $str;
		$self->{'incache'} = "";
	}

	$str =~ s/(\r|\n)\n?//og;
	for ($i = 0; $i < length($str); $i += 5) {
		$b = 0;
		if (substr($str, $i, 1) eq "z") {
			$i -= 4;
			$res .= pack("N", 0);
			next;
		}
		elsif ($isend && substr($str, $i, 6) =~ m/^(.{2,4})\~\>$/o) {
			$num = 5 - length($1);
			@c = unpack("C5", $1 . ("u" x (4 - $num)));     # pad with 84 to sort out rounding
			$i = length($str);
		}
		else {
			@c = unpack("C5", substr($str, $i, 5));
		}

		for ($j = 0; $j < 5; $j++) {
			$b *= 85;
			$b += $c[$j] - 33;
		}
		$res .= substr(pack("N", $b), 0, 4 - $num);
	}

	if (!$isend && $i > length($str)) {
		$self->{'incache'} = substr($str, $i - 5);
	}

	return $res;

}





# read data for LZWDecode
sub read_dat{

	#print "self = $self\n";
	my ($data_ref, $partial_code, $partial_bits, $code_length) = @_;
	$partial_bits = 0 unless defined $partial_bits;

	while ($partial_bits < $code_length) {
	
		if($partial_code){
			$partial_code = ($partial_code << 8) + unpack('C', $$data_ref);	
		}else{
			$partial_code =  unpack('C', $$data_ref);	
		}
		

		substr($$data_ref, 0, 1) = '';
		$partial_bits += 8;
	}

	my $code = $partial_code >> ($partial_bits - $code_length);
	$partial_code &= (1 << ($partial_bits - $code_length)) - 1;
	$partial_bits -= $code_length;

	return ($code, $partial_code, $partial_bits);
}




#LZWDecode
sub LZWDecode{

	#my $stream = shift;
	
	#my ($data, $is_last) =@_;
	my $self = {};
	$self->{'table'} = [map { pack('C', $_) } (0 .. 255, 0, 0)];
	$self->{'initial_code_length'} = 9;
	$self->{'code_length'} = 9;
	$self->{'clear_table'} = 256;
	$self->{'eod_marker'} = 257;
	$self->{'next_code'} = 258;


	

	#my $data = shift;
	my ($data, $obj_ref) =@_;
	my ($code, $partial_code, $partial_bits, $result);
	
	while ($data ne '' or $partial_bits) {
	
		#if($obj_ref->{"ref"} eq "30 0 obj"){
			#print "==> $data\n\n";
		#}
	
		($code, $partial_code, $partial_bits) = read_dat(\$data, $partial_code, $partial_bits, $self->{'code_length'});
		print "code = $code\n" unless $DEBUG eq "no";
		#$self->{'code_length'}++ if $self->{'next_code'} == (1 << $self->{'code_length'});

		if($self->{'next_code'} == (1 << $self->{'code_length'})){

			$self->{'code_length'}++
		}

		if ($code == $self->{'clear_table'}) {
			$self->{'code_length'} = $self->{'initial_code_length'};
			$self->{'next_code'} = $self->{'eod_marker'} + 1;
			next;
		}
		elsif ($code == $self->{'eod_marker'}) {
			last;
		}
		elsif ($code > $self->{'eod_marker'} ) {#&& $code< $self->{'next_code'}

			$self->{'table'}[$self->{'next_code'}] = $self->{'table'}[$code];


			#print "DEBUG tcnc= $table_code_nc ...\n";
			#my $table = $self->{'table'}[$code + 1];
			#print "DEBUG2 = $table ...\n";

			my $table_code = $self->{'table'}[$code];
			my $table_code_nc = $self->{'table'}[$self->{'next_code'}];
			my $table_code_plus = $self->{'table'}[$code + 1];

			# TODO Check Type FontDescriptor with FontFile object for buffer overflow. CVE 2010-2883
			if(!$table_code){
				print "/!\\ LZWdecode :: Potential buffer overflow in object $obj_ref->{ref} :: CVE 2010-2883!!\n" unless $DEBUG eq "yes";
				
				return $result;#.&LZWDecode($data,$obj_ref);;
			}

			#print "\n::DEBUG::\n code =$code\n next_code=$self->{next_code}\n table_code =$table_code\n table_next_code = $table_code_nc\n table_code_plus = $table_code_plus\n";

			$self->{'table'}[$self->{'next_code'}] .= substr($self->{'table'}[$code + 1], 0, 1);
			$result .= $self->{'table'}[$self->{'next_code'}];
			$self->{'next_code'}++;
		}
		else {
			
			print "code = $code :: next_code = $self->{next_code}!!\n" unless $DEBUG eq "no";
			$self->{'table'}[$self->{'next_code'}] = $self->{'table'}[$code];
			$result .= $self->{'table'}[$self->{'next_code'}];
			$self->{'next_code'}++;
		}
	}

	return $result;


	#return $out;
}



1;
__END__
