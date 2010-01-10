
typedef char rb_data_t;

#define RB_DATA_LEN    24

typedef struct buffer {
    char status;
    rb_data_t data[RB_DATA_LEN];
} rb_buffer_t;

#define LFRB_DATA_SIZE      ( sizeof(rb_data_t)*RB_DATA_LEN )

#define LFRB_BUFFER_TYPE    rb_buffer_t
#define LFRB_BUFFER_SIZE    ( sizeof(rb_buffer_t) )

#define LFRB_IS_AVAILABLE( el )     (el.status==0)
#define LFRB_MARK_AS_FILLED( el )   { (el).status=1; }
#define LFRB_MARK_AS_READ( el )     { (el).status=0; }
#define LFRB_DATA_PTR( el )         (el).data

