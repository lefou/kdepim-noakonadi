#include "article.h"
#include "articleinterceptor.h"

#include <q3valuelist.h>
#include <kstaticdeleter.h>

namespace Akregator
{

class ArticleInterceptorManager::ArticleInterceptorManagerPrivate 
{
    public:
        Q3ValueList<ArticleInterceptor*> interceptors;
};


ArticleInterceptorManager* ArticleInterceptorManager::m_self = 0;
KStaticDeleter<ArticleInterceptorManager> interceptormanagersd;

ArticleInterceptorManager* ArticleInterceptorManager::self()
{
    if (!m_self)
        interceptormanagersd.setObject(m_self, new ArticleInterceptorManager);
    return m_self;
}

ArticleInterceptorManager::~ArticleInterceptorManager() 
{
    delete d; 
    d = 0;
}

ArticleInterceptorManager::ArticleInterceptorManager() : d(new ArticleInterceptorManagerPrivate)
{}

void ArticleInterceptorManager::addInterceptor(ArticleInterceptor* interceptor)
{
    d->interceptors.append(interceptor);
}

void ArticleInterceptorManager::removeInterceptor(ArticleInterceptor* interceptor)
{
    d->interceptors.remove(interceptor);
}

Q3ValueList<ArticleInterceptor*> ArticleInterceptorManager::interceptors() const
{
    return d->interceptors;
}

}

