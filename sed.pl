#!/usr/bin/perl

my $s = join " ", @ARGV;

for (<STDIN>)
{
 eval $s;
 print;
}