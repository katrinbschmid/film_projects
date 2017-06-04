#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <stdio.h>
#include <assert.h>

template<class CLASS>
class Singleton
{
   public:

      Singleton()
      {
         assert(!m_Instance && "Already have an instance, thanks");
         m_Instance = static_cast<CLASS *>(this);
      }

      virtual ~Singleton()
      {
         assert(m_Instance && "No instance!");
         m_Instance = NULL;
      }

      static CLASS & Instance()
      {
         assert(m_Instance && "No instance!");
         return *m_Instance;
      }

      static bool IsValid()
      {
         return m_Instance ? true : false;
      }

   private:
      static CLASS * m_Instance;
};

#endif /* SINGLETON_H_ */

