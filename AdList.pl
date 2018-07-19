#!/usr/bin/perl

################# ͳ��һ����־��top��ѯ�ʵ�CTR
#use Data::Dumper;
#$date="0212";
$date = $ARGV[0];
$keyad = "log/".$date."/valid_keyad";
$cd_log = "log/".$date."/click_log";
$ie_log = "log/".$date."/pv_log";
#$cd_log = "/search/lihuabei/log/cd_ie_log/".$date.".cd_ie_log.10.10.67.55";
#$ie_log = "/search/lihuabei/log/sogou_ie_log/".$date.".sogou_ie_log";

$outfile = "log/".$date."/adlist";

@pid_black_list = ("sogou-tjsank");
foreach $pid (@pid_black_list)
{
	$pid_black_map{$pid} = 1;
}

# Adid to catalog Hashmap
print "load ad category from valid_keyad ...\n";
%adid_cate_map;

open FILE, $keyad or die "can't open $keyad\n";
while (<FILE>)
{
	chop;
	@tmp = split //;
	{
		$adid=$tmp[0];
		$cate=$tmp[2];
		if( ! exists $adid_cate_map{"$adid"})
		{
			$adid_cate_map{"$adid"}=$cate;
		}
	}
}
close FILE;
print "load adid_cate finished\n";

# pv record
print "loading pv records ...\n";
%sample_pv_map;

open FILE, $ie_log or die "can't open $ie_log.\n";
$cnt=0;
while (<FILE>)
{
	if ($cnt%100000 == 0)
	{
		print "current processing: $cnt\n";
	}
	$cnt++;
	chop;
	@tmp = split /\t/;
	{
		next if (exists $pid_black_map{$tmp[2]});
		$time = $tmp[0];
		$query = $tmp[12];
		# С��PID
		$PID = $tmp[2];
		# ���PID����Ϊsohu, sogou, sogou-*, others
		if ($PID =~ /^sohu$/)
		{
			$PID4 = "sohu";
		}
		elsif ($PID =~ /^sogou$/)
		{
			$PID4 = "sogou";
		}
		elsif ($PID =~ /^sogou-/)
		{
			$PID4 = "sogou-*";
		}
		else
		{
			$PID4 = "others";
		}
		@adid_list = split('\,', $tmp[5]);
		@accountid_list = split('\,', $tmp[6]);
		@flag_list = split('\,', $tmp[7]);
		@reserved_list = split('\,', $tmp[8]);
		@adkey_list = split('\,', $tmp[9]);
		for($i=0;$i<=$#adid_list;$i++)
		{
			# ���λ��
			$flg = $flag_list[$i]>>4 & 7;
			if($flg == 6)	#С����
			{
				$Region  = "С����";
				#$pv_pos = $reserved_list[$i]&2; # 0 ��ʾС������һλ��  2 ��ʾС�����ڶ�λ 
				#if ($pv_pos == 0)
				#{
				#	$pv_pos = 1;
				#}
				$pv_pos = (($reserved_list[$i]>>1)&15)+1; ## ���λ�� 
			}
			elsif($flg == 5 && $reserved_list[$i]%2 == 0 ) # right
			{
				$Region = "�Ҳ�";
				$page_num = ($reserved_list[$i]>>5)&3;
				$pv_pos = $page_num * 8 + (($reserved_list[$i]>>1)&15)+1; ## �Ҳ���λ�� 
			}
			elsif($flg == 5 && $reserved_list[$i]%2 == 1 ) # left 
			{
				$Region = "���";
				$page_num = ($reserved_list[$i]>>5)&3;
				$pv_pos = $page_num * 8 + (($reserved_list[$i]>>1)&15)+1; ## �Ҳ���λ�� 
			}
			else # ���Գ�����������Ĺ��
			{
				next;	
				#$Region = "etc";
				#$page_num = ($reserved_list[$i]>>5)&3;
				#$pv_pos = $page_num * 8 + (($reserved_list[$i]>>1)&15)+1; ## ���λ�� 
			}
			
			#��ù�����                   
			$tmp_adid = $adid_list[$i];
			if(exists $adid_cate_map{"$tmp_adid"})
			{
				$cate = $adid_cate_map{"$tmp_adid"};
			}
			else
			{
				$cate = "cate_not_found";
			}

			# hash the record
			$pv_sample = $query."\t".$adid_list[$i]."\t".$Region."\t".$pv_pos."\t".$accountid_list[$i]."\t".$PID."\t".$PID4."\t".$cate."\t".$adkey_list[$i];
			$sample_pv_map{"$pv_sample"}++;
		}
	}
}
close FILE;

# click record
print "loading click records ...\n";
%sample_click_map;
%sample_price_map;

open FILE, $cd_log or die "can't open cd.\n";
$cnt=0;
while (<FILE>)
{
	if ($cnt%10000 == 0)
	{
		print "current processing: $cnt\n";
	}
	$cnt++;

	chop;
	@tmp = split /\t/;
	# ����ĳЩpid
	next if (exists $pid_black_map{$tmp[2]});
	
	$time = $tmp[0];
	# С��PID
	$PID = $tmp[2];
	# ���PID����Ϊsohu, sogou, sogou-*, others
	if ($PID =~ /^sohu$/)
	{
		$PID4 = "sohu";
	}
	elsif ($PID =~ /^sogou$/)
	{
		$PID4 = "sogou";
	}
	elsif ($PID =~ /^sogou-/)
	{
		$PID4 = "sogou-*";
	}
	else
	{
		$PID4 = "others";
	}
	#if($time <= 1228967980)
	#{
	#	next;
	#}

	# position
	$flg = ($tmp[7]>>4) & 7;
	if ($flg==6 && $tmp[12] == 0) #ground
	{
		$Region = "С����";
		#$click_pos = $tmp[8]&2; ## 0 ��ʾС������һλ��  2 ��ʾС�����ڶ�λ
		#if ($click_pos == 0)
		#{
		#	$click_pos = 1;
		#}
		$click_pos = (($tmp[8]>>1)&15)+1; ## ���λ��
	}
	elsif ($flg==5 && $tmp[8]%2 == 0 && $tmp[12]==0)# right 
	{
		$Region = "�Ҳ�";
		$page_num = ($tmp[8]>>5)&3;
		$click_pos = $page_num * 8 + (($tmp[8]>>1)&15)+1; ## �Ҳ���λ��
	}
	elsif ($flg==5 && $tmp[8]%2 == 1 && $tmp[12]==0)# left
	{
		$Region = "���";
		$page_num = ($tmp[8]>>5)&3;
		$click_pos = $page_num * 8 + (($tmp[8]>>1)&15)+1; ## �Ҳ���λ��
	}
	else  # ���Գ�����������Ĺ��
	{
		next;
		#$Region = "etc";
		#$page_num = ($tmp[8]>>5)&3;
		#$click_pos = $page_num * 8 + (($tmp[8]>>1)&15)+1; ## ���λ��
	}

	#��ù�����                   
	$tmp_adid = $tmp[5];
	if(exists $adid_cate_map{"$tmp_adid"})
	{
		$cate = $adid_cate_map{"$tmp_adid"};
	}
	else
	{
		$cate = "cate_not_found";
	}
	
	# hash
	$query = $tmp[13];
	$query =~ tr/A-Z/a-z/; #��query�Ĵ�дתСд
	$click_sample = $query."\t".$tmp[5]."\t".$Region."\t".$click_pos."\t".$tmp[6]."\t".$PID."\t".$PID4."\t".$cate."\t".$tmp[9];
	if(exists $sample_pv_map{"$click_sample"})
	{
		$sample_click_map{"$click_sample"}++;
	}
	else 
	{
		$sample_pv_map{"$click_sample"}++;
		$sample_click_map{"$click_sample"}++;
	}
	# click price
	$sample_price_map{"$click_sample"} = $tmp[14];
}
close FILE;

print "generating ad date list ...";
open OUT, ">$outfile" or die "can't open acount_file_1.\n";
$cnt=0;
while (($key, $value) =  each %sample_pv_map)
{
	if ($cnt%100000 == 0)
	{
		print "current processing: $cnt\n";
	}
	$cnt++;
	if(exists $sample_click_map{"$key"})
	{
		$click = $sample_click_map{"$key"}; 
		$price = $sample_price_map{"$key"};
	}
	else
	{
		$click = 0; 
		$price = "na";
	}
	#if($value >= 10)
	{
		$ctr = $click/$value;
		print OUT "$value\t$click\t$ctr\t$key\t$price\n";
	}
}
close OUT;

