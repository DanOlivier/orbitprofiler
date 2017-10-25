//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once
#include <atomic>
#include <assert.h>

//-----------------------------------------------------------------------------
template < class T, unsigned BlockSize >
struct BlockChain
{
    struct Block
    {
        Block(BlockChain* a_Chain, Block* a_Prev)
            : m_Prev(a_Prev)
            , m_Next(nullptr)
            , m_Chain(a_Chain)
            , m_Size(0)
        {
        }

        ~Block()
        {
        }

        void Add(const T & a_Item)
        {
            if (m_Size == BlockSize)
            {
                if (m_Next == nullptr)
                {
                    m_Next = new Block(m_Chain, this);
                }

                m_Chain->m_Current = m_Next;
                ++m_Chain->m_NumBlocks;
                m_Next->Add(a_Item);
                return;
            }

            assert(m_Size < BlockSize);
            m_Data[m_Size] = a_Item;
            ++m_Size;
            ++m_Chain->m_NumItems;
        }

        Block*              m_Prev;
        Block*              m_Next;
        T                   m_Data[BlockSize];
        BlockChain*         m_Chain;
        std::atomic<unsigned>    m_Size;
    };

    struct BlockIterator
    {
        BlockIterator(Block* a_Block) : m_Block(a_Block)
        {
            m_Index = (m_Block && (m_Block->m_Size > 0)) ? 0 : -1;
        }

        T& operator*()
        {
            return m_Block->m_Data[m_Index];
        }

        bool operator!=(const BlockIterator & other) const
        {
            bool returnValue = m_Index != other.m_Index;
            return returnValue;
        }

        BlockIterator& operator++()
        {
            if (++m_Index == m_Block->m_Size)
            {
                if (m_Block->m_Next && m_Block->m_Next->m_Size > 0)
                {
                    m_Index = 0;
                    m_Block = m_Block->m_Next;
                }
                else
                {
                    m_Index = -1;
                }
            }

            return *this;
        }

        Block*      m_Block;
        unsigned    m_Index;
    };


    BlockChain() : m_NumBlocks(1), m_NumItems(0)
    {
        m_Root = m_Current = new Block(this, nullptr);
    }

    ~BlockChain()
    {
        // Find last block in chain
        while( m_Current->m_Next )
            m_Current = m_Current->m_Next;

        Block* prev = m_Current;
        while (prev)
        {
            prev = m_Current->m_Prev;
            delete m_Current;
            m_Current = prev;
        }
    }

    void push_back(const T & a_Item)
    {
        m_Current->Add(a_Item);
    }

    void push_back( const T* a_Array, unsigned a_Num )
    {
        for( unsigned i = 0; i < a_Num; ++i )
            m_Current->Add( a_Array[i] );
    }

    void push_back_n( const T & a_Item, unsigned a_Num )
    {
        for( unsigned i = 0; i < a_Num; ++i )
            m_Current->Add( a_Item );
    }

    void clear()
    {
        m_Root->m_Size = 0;
        m_Root->m_Next = nullptr;
        m_NumItems     = 0;
        m_NumBlocks    = 1;

        Block* prev = m_Current;
        while (prev!=m_Root)
        {
            prev = m_Current->m_Prev;
            delete m_Current;
            m_Current = prev;
        }

        m_Current = m_Root;
    }

    void Reset()
    {
        Block* blockPtr = m_Root;
        while( blockPtr )
        {
            blockPtr->m_Size = 0;
            blockPtr = blockPtr->m_Next;
        }

        m_NumItems = 0;
        m_NumBlocks = 1;
        m_Current = m_Root;
    }

    bool keep(unsigned a_MaxElems )
    {
        bool hasDeleted = false;
        a_MaxElems = std::max( BlockSize + 1, a_MaxElems );

        while( m_NumItems > a_MaxElems )
        {
            --m_NumBlocks;
            m_NumItems -= BlockSize;

            m_Root = m_Root->m_Next;

            assert( m_Root->m_Prev );
            assert( m_Root->m_Prev != m_Current );

            delete m_Root->m_Prev;
            m_Root->m_Prev = nullptr;
            hasDeleted = true;
        }

        return hasDeleted;
    }

    unsigned size() const { return m_NumItems; }
    
    T* SlowAt(unsigned a_Index )
    {
        if( a_Index < m_NumItems && a_Index >= 0 )
        {
            unsigned count = 1;
            Block* block = m_Root;
            while( count * BlockSize < a_Index && block && block->m_Next )
            {
                block = block->m_Next;
                ++count;
            }

            unsigned index = a_Index % BlockSize;
            return &block->m_Data[index];
        }

        return nullptr;
    }
   
    BlockIterator begin(){ return BlockIterator(m_Root); }
    BlockIterator end(){ return BlockIterator(nullptr); }

    Block*      m_Root;
    Block*      m_Current;
    std::atomic<unsigned> m_NumBlocks;
    std::atomic<unsigned> m_NumItems;
};
