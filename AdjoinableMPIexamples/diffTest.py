#!/usr/bin/env python
import sys
import difflib
import re

class CompException(Exception):
    
    def __init__(self,message,controlLine,chunkLines):
        self.msg=message
        self.controlLine=controlLine
        self.chunkLines=chunkLines

class MissMatch:
    
    def __init__(self,lineNumber, numberPair,linePair):
        self.lineNumber=lineNumber
        self.numberPair=numberPair
        self.linePair=linePair

    def __str__(self):
        indent="   "
        rStr=indent+"line:"+str(self.lineNumber)+" numbers: "+str(self.numberPair[0])+" vs. "+str(self.numberPair[1])+"\n"
        rStr+=indent+"< "+self.linePair[0]+"\n"
        rStr+=indent+"---\n"
        rStr+=indent+"> "+self.linePair[1]
        return rStr

class DiffResult:

    # limits: <key>:(<type>,<initValue>,<defaultLimit>,<limitComparison>)
    ourLimits={'floatMinMatchDigits':(int,sys.maxint,0,min),
               'floatMaxAbsoluteDiscrepancy':(float,0.0,float('inf'),max),
               'floatMaxRelativeDiscrepancy':(float,0.0,float('inf'),max)}
    
    def __init__(self,excludePrefixes):
        self.excludePrefixes=excludePrefixes
        self.intMissMatch=None
        for key,limitTuple in DiffResult.ourLimits.items():
            vars(self)[key]=limitTuple[1]
            vars(self)[key+"Error"]=limitTuple[2]
            vars(self)[key+"MissMatch"]=None    

    def __str__(self):
        rStr=''
        for key in DiffResult.ourLimits.keys():
            if (not vars(self)[key+"MissMatch"] is None):
                rStr+=key+": "+str(vars(self)[key])+"\n"
                rStr+=str(vars(self)[key+"MissMatch"])+"\n"
        if self.intMissMatch:
            rStr+="integer mismatch "+str(self.intMissMatch)+'\n'
        if self.excludePrefixes:
            rStr+="excluded lines with the following prefixes: "+str(self.excludePrefixes)
        return rStr

    def checkExceedsLimits(self):
        rValue=0
        for key,limitTuple in DiffResult.ourLimits.items():
            if (vars(self)[key+"Error"]!=limitTuple[3](vars(self)[key],vars(self)[key+"Error"])):
                sys.stderr.write("ERROR: observed value "+str(vars(self)[key])+" for "+key+" violates error limit "+str(vars(self)[key+"Error"])+"\n")
                rValue+=1
        return rValue

    def fixOOM(self,decPointPos,mantExpPairs):
        ''' things like 9.995E+2 vs 1.0005E+3 '''
        if (abs((decPointPos[0]-decPointPos[1])-(mantExpPairs[1][1]-mantExpPairs[0][1]))>1) :
            return False;
        modPairIndex=-1
        for pos,mantExpPair in enumerate(mantExpPairs):
            if ((mantExpPair[0][0]=='-' and mantExpPair[0][1]=='9')
                or
                mantExpPair[0][0]=='9'):
                modPairIndex=pos
                break
        if modPairIndex==-1:
            return False
        mantExpPairs[modPairIndex][0]=mantExpPairs[modPairIndex][0][:decPointPos[modPairIndex]]+mantExpPairs[modPairIndex][0][decPointPos[modPairIndex]+1:]
        mantExpPairs[modPairIndex][0]='0.'+mantExpPairs[modPairIndex][0]
        pointShift=decPointPos[modPairIndex]
        decPointPos[modPairIndex]=1
        mantExpPairs[modPairIndex][1]=mantExpPairs[modPairIndex][1]+pointShift
        return True
        
    def matchDigits(self,lineNumber,numberPair,linePair):
        def expSplitter(aString):
            ''' returns a pair of the mantissa as a string and the exponent as an integer '''
            expPos=filter(lambda l: l>-1,map(lambda l: aString.find(l),['e','d','E','D']))
            if expPos:
                return [aString[:expPos[0]],int(aString[expPos[0]+1:])]
            return [aString,0]
        # strip leading spaces
        sNumberPair=map(lambda l: l.lstrip(),numberPair)
        # strip leading plus
        sNumberPair=map(lambda l: l.lstrip('+'),sNumberPair)
        # strip leading zeroes up to the last before the decimal point
        def stripHelper(n):
            nSplit=n.split('.')
            r=nSplit[0].lstrip('0')
            if (r==''):
                r='0'
            if len(nSplit)>1:
                r+='.'+nSplit[1]
            return r
        sNumberPair=map(stripHelper,numberPair)
        # find decimal point position 
        decPointPos=map(lambda l: l.find('.'),sNumberPair)
        # get exponents
        mantExpPairs=map(expSplitter,sNumberPair)
        # check decimal point position vs exponents
        if (decPointPos[0]-decPointPos[1]!=mantExpPairs[1][1]-mantExpPairs[0][1]) :
            if (not self.fixOOM(decPointPos,mantExpPairs)) : 
                self.floatMinMatchDigits=0
                self.floatMinMatchDigitsMissMatch=MissMatch(lineNumber,numberPair,linePair)
                return
        # extract mantissas
        mantissas=map(lambda l: l[0],mantExpPairs)
        # if the exponents vs decimal points didn't  match we would have returned
        # now we can remove the point from the mantissas
        mantissas=map(lambda l: filter(lambda l1: l1!='.',l),mantissas)
        try: 
            # find the nonmatching position
            pos=map(lambda l: l[0]==l[1],zip(mantissas[0],mantissas[1])).index(False)
            if (abs(int(mantissas[0][pos])-int(mantissas[1][pos]))==1): # one off
                # dealing with a case like 0.99940E-01 vs 1.00030E-01
                # conservatively up the matching position count as in the logic below
                # (equals to  3 in the example case) because
                # we could shift by some delta (in the example say 0.0007E-01 and that
                # would give 1.00010E-01 vs 1.00100E-01)
                # which would yield a higher matching count
                if (mantissas[0][pos]>mantissas[1][pos]):
                    while((pos+1)<min(map(len,mantissas)) and mantissas[1][pos+1]=='9' and mantissas[0][pos+1]=='0') :
                        pos+=1
                else: 
                    while((pos+1)<min(map(len,mantissas)) and mantissas[0][pos+1]=='9' and mantissas[1][pos+1]=='0') :
                        pos+=1
            if ((not (self.floatMinMatchDigits is None) and pos<self.floatMinMatchDigits)
                or
                self.floatMinMatchDigits is None):
                self.floatMinMatchDigits=pos
                self.floatMinMatchDigitsMissMatch=MissMatch(lineNumber,numberPair,linePair)
        except ValueError, e : # there is no mismatch
            pass

    # control line looks like this: @@ -23,4 +23,4 @@  or this @@ -23 +23 @@ if one line changes
    __controlLinePatn=re.compile("@@ -(?P<lLine>\d+)(?P<lCount>[,\d]*) \+(?P<rLine>\d+)(?P<rCount>[,\d]*) @@")
    
    def handleChunk(self,controlLine,chunkLines):
        chunkLines=map(lambda l:l[:-1],chunkLines)
        controlLine=controlLine.strip()
        match=re.match(DiffResult.__controlLinePatn,controlLine)
        if (match.group('lLine')!=match.group('rLine') or match.group('lCount')!=match.group('rCount')) : 
            raise CompException("output format change indicated by control line",controlLine,chunkLines)
        chunkLines=map(lambda l: l[1:],chunkLines) # strip of leading indicators
	fromExtent=1
	if (match.group('lCount')!='') : 
	    fromExtent=int(match.group('lCount')[1:])
	toStart=int(match.group('rLine'))
        zipped=zip(chunkLines[:fromExtent],chunkLines[fromExtent:]) # zip them
        for offset,pair in enumerate(zipped):
            # skip lines starting with any of the excludePrefixes
            if (any(map(lambda p: pair[0][:len(p)]==p and pair[1][:len(p)]==p,
                        self.excludePrefixes))):
                continue
            zippedNumbers=zip(pair[0].split(),pair[1].split())
            for numberPair in zippedNumbers:
		if (numberPair[0]==numberPair[1]):
		    continue
                doneWithNumberPair=False
                try:
                    map(lambda l: int(l,10),numberPair)
                    doneWithNumberPair=True
                    if numberPair[0]!=numberPair[1]:
                        if (not self.intMissMatch):
                            self.intMissMatch=MissMatch(toStart+offset,numberPair,pair)
                except ValueError, e:
                    pass
                try:
                    floatPair=map(float,numberPair)
                    doneWithNumberPair=True
                    if (floatPair[0]!=floatPair[1]):
                        if (abs(floatPair[0]-floatPair[1])>self.floatMaxAbsoluteDiscrepancy):
                            self.floatMaxAbsoluteDiscrepancy=abs(floatPair[0]-floatPair[1])
                            self.floatMaxAbsoluteDiscrepancyMissMatch=MissMatch(toStart+offset,numberPair,pair)
                        # it doesn't make sense to check matching digits or relative error if one of the numbers is 0
                        if (min(abs(floatPair[0]),abs(floatPair[1]))!=0.0):
                            relD=((abs(floatPair[0])-abs(floatPair[1]))/min(abs(floatPair[0]),abs(floatPair[1])))
                            if (relD>self.floatMaxRelativeDiscrepancy):
                                self.floatMaxRelativeDiscrepancy=relD
                                self.floatMaxRelativeDiscrepancyMissMatch=MissMatch(toStart+offset,numberPair,pair)
                            self.matchDigits(toStart+offset,numberPair,pair)
                except ValueError, e:
                    pass
                if (not  doneWithNumberPair):
                    raise CompException("output format change for pair ("+numberPair[0]+","+numberPair[1]+")",controlLine,chunkLines)

def main():
    from optparse import OptionParser
    opt = OptionParser(usage='%prog [options] <file1> <file2> ...')
    defaultExcludes=[]
    opt.add_option('','--excludePrefixes',dest='excludePrefixes',
                   metavar='LIST',
                   help="python LIST of strings containing prefixes of lines to be excluded from the comparison (defaults to "+str(defaultExcludes)+")",
                   default=defaultExcludes)
    opt.add_option('','--errorOverLimits',dest='errorOverLimits',
                   metavar='DICT',
                   help="where DICT is a python dictionary containing numerical values for the following keys >"\
                   +','.join(DiffResult.ourLimits.keys())\
                   +"< (defaults to "+repr(dict(map(lambda l: (l[0],l[1][2]),DiffResult.ourLimits.items())))\
                   +" ) that will trigger an error message and nonzero return code if the specified limits are crossed",
                   default=None)
    (options, args) = opt.parse_args()
    for f in args:
        aDiffResult=DiffResult(options.excludePrefixes)
        if options.errorOverLimits:
            try: 
                limitsDict=eval(options.errorOverLimits)
                for (key,value) in limitsDict.items():
                    if not key in DiffResult.ourLimits.keys():
                        opt.error("invalid key >"+key+"< for --errorOverLimits flag; must be one of "+','.join(DiffResult.ourLimits.keys()))
                    numVal=None
                    try:
                        numVal=DiffResult.ourLimits[key][0](value)
                        vars(aDiffResult)[key+"Error"]=numVal
                    except ValueError, e:
                        opt.error("invalid value >"+value+"< for key >"+key+"< for --errorOverLimits flag; "+e.msg)
            except SyntaxError, e:
                opt.error("Syntax error parsing argument of --errorOverLimits:\n"+e.text+"\n"+" "*(e.offset-1)+"^")
        chunkLines=[]
        controlLine=''
        try:
            fromFile=open(f,"r")
            toFile=open(f+".ref","r")
            diff=difflib.unified_diff(fromFile.readlines(),
                                      toFile.readlines(),n=0)
            for line in diff:
                if line[:3] in ['---','+++']:
                    continue
                if line[:3] == '@@ ':
                    if chunkLines:
                        # note - controlLine is still the previous one
                        aDiffResult.handleChunk(controlLine,chunkLines)
                    # start new chunk
                    controlLine=line
                    chunkLines=[]
                    continue
                if line[0] in ['-','+']:
                    chunkLines.append(line)
            if chunkLines:
                aDiffResult.handleChunk(controlLine,chunkLines)
        except CompException, e:
            sys.stderr.write('ERROR: '+e.msg+'\n'+e.controlLine+'\n'+'\n'.join(e.chunkLines))
            return 1
        if (str(aDiffResult)):
            sys.stdout.write(str(aDiffResult)+"\n")
        rc=aDiffResult.checkExceedsLimits()
        if (rc):
            return(rc)
        sys.stdout.write(f+" reference check OK\n")
        
if __name__ == "__main__":
    sys.exit(main())
                             
