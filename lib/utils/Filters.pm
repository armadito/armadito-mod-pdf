package Filters;

use strict;
use Compress::Zlib;

my $DEBUG = "no";


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

	#my $tmp;
	#my $inflation;

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

	#($out, $status2) = $y->inflate($stream) or print "Error inflating the stream\n";
	#print "status = $status2 ::".$y->msg()."\n";

	# 
	#my @arr = split('',$stream);
	#print "arr == @arr\n";
	#foreach(@arr){
	#	$out .= $y->inflate($_) or die "Error inflating the stream\n";		
	#}
	#print "status = $status2 ::".$y->msg()."\n";
	#print "OUT = $out\n";

	return $deflation;
}


#AsciiHexDecode
sub AsciiHexDecode{

	my $stream = shift;
	my $out;
	
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
		$partial_code = ($partial_code << 8) + unpack('C', $$data_ref);

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
				print "/!\ LZWdecode :: Potential buffer overflow in object $obj_ref->{ref} :: CVE 2010-2883!!\n" unless $DEBUG eq "no";
				return $result;
			}
			#if($obj_ref->{""} ~= ){

			#}


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
