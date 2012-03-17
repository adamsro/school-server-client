import os
import sys
import getopt
import urllib2
import re
import math

#from pudb import set_trace; set_trace()

def makecourse(term, course):
    try:
        try:
            opts, args = getopt.getopt(sys.argv[1:], "ho:v", ["help", "output="])
        except getopt.GetOptError, err:
            print str(err) 
        os.makedirs('%s/%s' % (term, course))
        os.chdir('%s/%s' % (term, course))
        os.mkdir('assignments')
        os.mkdir('examples')
        os.mkdir('exams')
        os.mkdir('lecture_notes')
        os.mkdir('submissions')
    except:
        return False
    return True

def get_phone_numbers(url):
    out = []
    prog = re.compile("(\d{3}[-\.\s]??\d{3}[-\.\s]??\d{4}|\(\d{3}\)\s*\d{3}[-\.\s]??\d{4}|\d{3}[-\.\s]??\d{4})")
    f = urllib2.urlopen(url)
    for line in f:
        temp =prog.findall(line)
        if len(temp) > 0:
            out.append(temp)
    return out

def _is_prime(n):
    n = abs(int(n))
    if n == 2: return True
    if not n & 1: return False # if even, not prime
    for x in range(3, int(n**0.5)+1, 2):
        if n % x == 0:
            return False
    return True

def get_1001st_prime():
    i = 1; primes = 0
    while(True):
        if _is_prime(i):
            primes += 1
        if primes == 1001:
            return i
        i += 2

def greatest_product():
    num = """73167176531330624919225119674426574742355349194934
96983520312774506326239578318016984801869478851843
85861560789112949495459501737958331952853208805511
12540698747158523863050715693290963295227443043557
66896648950445244523161731856403098711121722383113
62229893423380308135336276614282806444486645238749
30358907296290491560440772390713810515859307960866
70172427121883998797908792274921901699720888093776
65727333001053367881220235421809751254540594752243
52584907711670556013604839586446706324415722155397
53697817977846174064955149290862569321978468622482
83972241375657056057490261407972968652414535100474
82166370484403199890008895243450658541227588666881
16427171479924442928230863465674813919123162824586
17866458359124566529476545682848912883142607690042
24219022671055626321111109370544217506941658960408
07198403850962455444362981230987879927244284909188
84580156166097919133875499200524063689912560717606
05886116467109405077541002256983155200055935729725
71636269561882670428252483600823257530420752963450
""".replace('\n','')
    allcombo = [substr for x in range(len(num))]

def find_multiples():
    return [x for x in range(1000) if x % 3 == 0 or x % 5 == 0] 


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print 'usage: part2 function [arg1] [arg2] ...'

    if sys.argv[1] == 'makecourse':
        makecourse(sys.argv[2], sys.argv[3])

    elif sys.argv[1] == 'get_phone_numbers':
        if sys.argv[2] is None:
            exit("missing url arg")
        print get_phone_numbers(sys.argv[2])

    elif sys.argv[1] == 'get_1001st_prime':
        print get_1001st_prime()

    elif sys.argv[1] == 'greatest_product':
        print greatest_product()

    elif sys.argv[1] == 'find_multiples':
        print find_multiples()
