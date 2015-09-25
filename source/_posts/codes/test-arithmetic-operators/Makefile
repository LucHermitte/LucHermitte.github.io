test-arithmetic-operators-left%: test-arithmetic-operators.cpp test-arithmetic-operators.h
	$(CXX) $(CXXFLAGS) -DMETHOD_LEFT=$(shell echo $@ | sed "s#.*\(.\)#\1#") -o $@ $<
