#ifndef _LEAR_LIST_POST_PROCESS_RESULT_H_
#define _LEAR_LIST_POST_PROCESS_RESULT_H_

#include <fstream>
#include <lear/classifier/ppresult.h>
namespace lear {
    struct List_PPResult : PostProcessResult {
        typedef PostProcessResult                       Parent;

        List_PPResult(const std::string& listfile_, const int marker=1, const bool hardtest=false);
        virtual ~List_PPResult() {}

        /** Write detection results, must be invoked after invoking
        * operator() on all tested window to write corresponding detection 
        * results. 
        */
        virtual void write(
            const_iterator s, const_iterator e, 
            const std::string filename);

        virtual std::string toString() { 
            return "List Post Process Result";
        }
        protected:
            std::ofstream to;
	    std::string to1;
            const int marker;
	    const bool hardtest;
    };

}
#endif // _LEAR_LIST_POST_PROCESS_RESULT_H_
