/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body dbms_lock is


--  Request a lock with the given mode. Note that this routine is
--    overloaded based on the type of its first argument.  The
--    appropriate routine is used based on how it is called.
--    If a deadlock is detected, then an arbitrary session is
--    chosen to receive deadlock status.
--    ***NOTE*** When running both multi-threaded server (dispatcher) AND
--    parallel server, a multi-threaded "shared server" will be
--    bound to a session during the time that any locks are held.
--    Therefore the "shared server" will not be shareable during this time.
--  Input parameters:
--    id
--      From 0 to 1073741823.  All sessions that use the same number will
--      be referring to the same lock. Lockids from 2000000000 to
--      2147483647 are accepted by this routine.  Do not use these as
--      they are reserved for products supplied by Oracle Corporation.
--    lockmode
--    公矫等促.
--    timeout
--    公矫等促.
--    release_on_commit
--    公矫等促.
--  Return value:
--    0 - success
--    1 - timeout
--    3 - parameter error
--    4 - already own lock specified by 'id' or 'lockhandle'

function  request(id in integer,
                  lockmode in integer default x_mode,
                  timeout in integer default maxwait,
                  release_on_commit in boolean default FALSE)
                  return integer
as
    ret integer;
begin
    ret := user_lock_request( id );

    return ret;
end request;


--  Release a lock previously aquired by 'request'. Note that this routine
--    is overloaded based on the type of its argument.  The
--    appropriate routine is used based on how it is called.
--  Input parameters:
--    id
--      From 0 to 1073741823.
--  Return value:
--    0 - success
--    3 - parameter error
--    4 - don't own lock specified by 'id' or 'lockhandle'
--
function release(id in integer) return integer
as
    ret integer;
begin
    ret := user_lock_release ( id );

    return ret;
end release;


--  Suspend the session for the specified period of time.
--  Input parameters:
--    seconds
--      In seconds, currently the maximum resolution is in hundreths of
--      a second (e.g., 1.00, 1.01, .99 are all legal and distinct values).
procedure sleep(seconds in number)
as
    dummy integer;
begin
    dummy := sp_sleep( seconds );
end sleep;

end dbms_lock;
/

