//
//  getch.h
//  getch
//
//  Created by Eduardo Almeida on 22/05/13.
//  Copyright (c) 2013 Bitten Apps. All rights reserved.
//

#ifndef getch_getch_h
#define getch_getch_h



#ifdef __cplusplus // preprocessor <--- When c++ code call c-function.
extern "C"{
#endif

int getch(void);

#ifdef __cplusplus
}
#endif

#endif
