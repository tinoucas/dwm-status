NAME		=  dwm_status

SRCS		=  status.c

OBJS		=	$(SRCS:.c=.o)

RM			=	rm -f

CP			=	cp -f

INSTALLPATH	=	~/bin/

CFLAGS		=	-O3

LDFLAGS		= -lX11 -lpthread


$(NAME)	:	$(OBJS)
			$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

all		:	$(NAME)

clean	:
			$(RM) $(OBJS) $(NAME)

re		:	fclean all

install	:	$(NAME)
			$(CP) $(NAME) $(INSTALLPATH)

