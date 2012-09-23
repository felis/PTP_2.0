#if !defined(__SIMPLEFIFO_H__)
#define __SIMPLEFIFO_H__

template <class TYPE, const uint8_t SIZE>
class SimpleFIFO
{
    TYPE      theBuffer[SIZE];
    uint8_t   tail, head;
    
private: 
    void inc(uint8_t &val)
    {
        val ++;
        
        if (val >= SIZE)
            val = 0;
    };
public:
    SimpleFIFO() :
        tail(0),
        head(0)
    {
    };
    uint8_t Size()
    {
        if (tail == head)
            return 0;
            
        if (tail > head)
            return (tail - head);
        else
            return (SIZE - head + tail);
    };
    void Empty()
    {
        tail = head = 0;
    };
    void Push(TYPE val)
    {
        if (Size() >= SIZE-1)
            return;
            
        theBuffer[tail] = val;
        inc(tail);
    };
    TYPE Pop()
    {
        if (head == tail) 
            return (TYPE)0;
        
        TYPE ret = theBuffer[head];
        inc(head);
        return ret;
    };
};


#endif // __SIMPLEFIFO_H__