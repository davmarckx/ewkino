

class EventChecker : 
    def __init__( self, input_file_name ):
        with open( input_file_name ) as f:
            event_list = []
            for line in f.readlines():
                line = line.split(':')[-1]
                event_tags = tuple( ( int( entry ) for entry in line.split('/') ) )
                event_list.append( event_tags )
        self.event_set = set( event_list )

    def isPresent( self, event_tags ):
        return ( event_tags in self.event_set )
                


if __name__ == '__main__' :
    checker = EventChecker( 'event_list_2016.txt' )
    assert checker.isPresent( (1,81,13123) )
    assert not checker.isPresent( (2,81,13123) )
    print( 'pass small test' )
