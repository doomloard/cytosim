// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "glossary.h"
#include "iowrapper.h"
#include "key_list.h"
#include "aster.h"
#include "solid.h"
#include <iostream>
#include <set>


/**
 @copydetails Simul::reportW
 @copydetails Simul::reportWW
 */
void Simul::report(std::ostream& out, std::string const& str, Glossary& opt) const
{
    size_t pos = str.find(":");
    if ( std::string::npos == pos )
        report(out, str.substr(0, pos), std::string(), opt);
    else
        report(out, str.substr(0, pos), str.substr(pos+1), opt);
    out << std::endl;
}


/**
 
 WHAT                |   output
 --------------------|------------------------------------------------------
 `organizer`         | Position of the center of asters and other organizers
 `bead`              | Position of beads
 `couple`            | Number and state of couples
 `fiber`             | Length and position of the ends of fibers
 `single`            | Number and state of singles
 `solid`             | Position of center and first point of solids
 `sphere`            | Position of center and first point of spheres
 `time`              | Time
 `parameters`        | All object properties
 
 
 WHAT:WHO            |   output
 --------------------|--------------------------------------------------------------------
 `fiber:lengths`     | Average length and standard deviation of fibers
 `fiber:dynamics`    | Number of fiber classified by PLUS_END Dynamic state
 `fiber:points`      | coordinates of model points of all fibers
 `fiber:speckles`    | coordinates of points randomly distributed along all fibers
 `fiber:segments`    | information about lengths of segments, number of kinks
 `fiber:ends`        | Positions and dynamic states of all fiber ends
 `fiber:forces`      | Position of model points and Forces acting on model points
 `fiber:tensions`    | Internal stress along fibers
 `fiber:clusters`    | Clusters made of fibers connected by Couples
 `bead:all`          | Position of beads
 `bead:singles`      | Number of Beads with no single attached, 1 single attached etc.
 `single:all`        | Position and force of singles
 `single:NAME`       | Position of singles of class NAME
 `couple:all`        | Position of couples
 `couple:NAME`       | Position of couples of class NAME
 
 */
void Simul::report(std::ostream& out, std::string const& what, std::string const& who, Glossary& opt) const
{
    if ( what == "fiber" )
    {
        if ( who.empty() )
            return reportFiber(out);
        if ( who == "ends" )
            return reportFiberEnds(out);
        if ( who == "points" )
            return reportFiberPoints(out);
        if ( who == "speckles" )
        {
            real len = 1;
            opt.set(len, "interval");
            return reportFiberSpeckles(out, len);
        }
        if ( who == "segments" )
            return reportFiberSegments(out);
        if ( who == "lengths" )
            return reportFiberLengths(out);
        if ( who == "length_distribution" )
        {
            real len = 1, max = 10;
            opt.set(len, "interval");
            opt.set(max, "max");
            return reportFiberLengthDistribution(out, len, max);
        }
        if ( who == "tensions" )
            return reportFiberTension(out, opt);
        if ( who == "dynamics" )
            return reportFiberDynamic(out);
        if ( who == "forces" )
            return reportFiberForces(out);
        if ( who == "clusters" )
            return reportClusters(out, 1);
        throw InvalidSyntax("I only know fiber: ends, points, speckles, segments, dynamics, lengths, length_distribution, tensions, forces, clusters");
    }
    if ( what == "bead" )
    {
        if ( who.empty() )
            return reportBeadPosition(out);
        if ( who == "singles" )
            return reportBeadSingles(out);
        if ( who == "positions" || who == "all" )
            return reportBeadPosition(out);
        throw InvalidSyntax("I only know bead: all, singles");
    }
    if ( what == "solid" )
    {
        if ( who.empty() )
            return reportSolid(out);
        throw InvalidSyntax("I only know `solid'");
    }
    if ( what == "space" )
    {
        if ( who.empty() )
            return reportSpace(out);
        throw InvalidSyntax("I only know `space'");
    }
    if ( what == "sphere" )
    {
        if ( who.empty() )
            return reportSphere(out);
        throw InvalidSyntax("I only know `sphere'");
    }
    if ( what == "single" )
    {
        if ( who.empty() )
            return reportSingle(out);
        if ( who == "positions" || who == "all" )
            return reportSinglePosition(out);
        if ( who == "forces" )
            return reportSingleForce(out);
        else
            return reportSinglePosition(out, who);
        throw InvalidSyntax("I only know single: all, forces, NAME");
    }
    if ( what == "couple" )
    {
        if ( who.empty() )
            return reportCouple(out);
        else if ( who == "positions" || who == "all" )
            return reportCouplePosition(out);
        else
            return reportCouplePosition(out, who);
        throw InvalidSyntax("I only know couple: all, NAME");
    }
    if ( what == "organizer" )
    {
        if ( who.empty() )
            return reportOrganizer(out);
        throw InvalidSyntax("I only know `organizer'");
    }
    if ( what == "time" )
    {
        if ( who.empty() )
            return reportTime(out);
        throw InvalidSyntax("I only know `time'");
    }
    if ( what == "parameters" )
    {
        if ( who.empty() )
            return writeProperties(out, false);
        throw InvalidSyntax("I only know `parameters'");
    }
    if ( what == "custom" )
        return reportCustom(out);

    throw InvalidSyntax("I do not know how to write `"+what+"'");
}



void Simul::reportTime(std::ostream& out) const
{
    out << std::left << std::setw(9) << simTime() << std::endl;
}

//------------------------------------------------------------------------------
#pragma mark -

inline void write(std::ostream& out, Object * obj)
{
    out << obj->property()->index() << " " << std::setw(8) << obj->number();
}


bool select_fiber(Fiber const* fib, void* prop)
{
    return ( fib->prop == prop );
}

/**
 Export average length and standard-deviation for each class of fiber
 */
void Simul::reportFiberLengths(std::ostream& out) const
{
    out << "%          fiber    count  avg_len  std_dev      total" << std::endl;

    
    unsigned cnt;
    real avg, dev;
    
    PropertyList plist = properties.find_all("fiber");
    
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        FiberProp * fp = static_cast<FiberProp*>(*ip);
        
        fibers.infoLength(cnt, avg, dev, select_fiber, fp);
        out << std::fixed;
        out.precision(3);
        out << std::setw(16) << fp->name();
        out << " " << std::setw(8) << cnt;
        out << " " << std::setw(8) << avg;
        out << " " << std::setw(8) << dev;
        out << " " << std::setw(10) << avg*cnt;
        out << std::endl;
    }
}


/**
 Export average length and standard-deviation for each class of fiber
 */
void Simul::reportFiberLengthDistribution(std::ostream& out, real delta, real max) const
{
    assert_true( max > 0 );
    assert_true( delta > 0 );
    
    const int nbin = ceil( max / delta );
    unsigned * cnt = new unsigned[nbin+1];
    
    PropertyList plist = properties.find_all("fiber");
    
    out << "% fiber_name";
    for ( unsigned u = 0; u <= nbin; ++u )
        out << " " << std::setw(8) << std::setprecision(4) << delta * u;
    out << std::endl;
    
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        FiberProp * fp = static_cast<FiberProp*>(*ip);
        
        for ( unsigned u = 0; u <= nbin; ++u )
            cnt[u] = 0;
        
        for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
        {
            if ( obj->property() == fp )
            {
                unsigned u = floor( obj->length() / delta );
                if ( u < nbin )
                    ++cnt[u];
                else
                    ++cnt[nbin];
            }
        }
        
        out << std::setw(12) << fp->name();
        for ( unsigned u = 0; u <= nbin; ++u )
            out << " " << std::setw(8) << cnt[u];
        out << std::endl;
    }
    delete(cnt);
}


void Simul::reportFiberSegments(std::ostream& out) const
{
    out << "%fiber_name amount joints kinks min_seg max_seg" << std::endl;
    
    PropertyList plist = properties.find_all("fiber");
    
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        FiberProp * fp = static_cast<FiberProp*>(*ip);
        
        unsigned cnt, joints, kinks;
        real mn = 0, mx = 0;
        
        fibers.infoSegments(cnt, joints, kinks, mn, mx, select_fiber, fp);
        out << std::fixed;
        out.precision(5);
        out << std::setw(10) << fp->name() << " " << std::setw(6) << cnt;
        out << " " << std::setw(6) << joints << " " << std::setw(5) << kinks;
        out << " " << std::setw(7) << mn << " " << std::setw(7) << mx << std::endl;
    }
}


/**
 Export number of fiber, classified according to dynamic state of one end
 */
void Simul::reportFiberDynamic(std::ostream& out, FiberEnd end) const
{
    static int top = 5;
    std::vector<int> cnt(top);
    int sum = 0;
    
    for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
    {
        ++sum;
        int s = obj->dynamicState(end);
        if ( 0 <= s && s < top )
            ++cnt[s];
    }
    
    if ( end == PLUS_END )
        out << " plus_end:" << std::setw(8) << std::left << sum;
    else if ( end == MINUS_END )
        out << "minus_end:" << std::setw(8) << std::left << sum;
    
    for ( int ii = 0; ii < top; ++ii )
        out << " " << ii << ":" << std::setw(5) << std::left << cnt[ii];
    out << std::endl;
}

/**
 Export number of fiber, classified according to dynamic state of one end
 */
void Simul::reportFiberDynamic(std::ostream& out) const
{
    out << "% fiber_end:total state:amount ..." << std::endl;
    reportFiberDynamic(out, PLUS_END);
    reportFiberDynamic(out, MINUS_END);
}


/**
 Export length, position and directions at center of fibers
 */
void Simul::reportFiber(std::ostream& out) const
{
    out << "% class id length center_position center_direction" << std::endl;
    
    PropertyList plist = properties.find_all("fiber");
    
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        FiberProp * fp = static_cast<FiberProp*>(*ip);
        
        out << "% " << fp->name() << std::endl;
        out << std::fixed;
        out.precision(5);
        
        for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
        {
            if ( obj->property() == fp )
            {
                write(out, obj);
                out << "  " << std::setw(9) << obj->length();
                out << "  " << std::setw(9) << obj->posEnd(CENTER);
                out << "  " << std::setw(9) << obj->dirEnd(CENTER);
                out << std::endl;
            }
        }
    }
}



/**
 Export dynamic state, positions and directions of fiber ends
 */
void Simul::reportFiberEnds(std::ostream& out) const
{
    out << "% class id   length stateM positionM directionM  stateP positionP directionP" << std::endl;
    out << std::fixed;
    out.precision(5);
    
    for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
    {
        write(out, obj);
        out << "  " << std::setw(9) << obj->length();
        out << "  " << obj->dynamicState(MINUS_END);
        out << "  " << std::setw(9) << obj->posEnd(MINUS_END);
        out << "  " << std::setw(9) << obj->dirEnd(MINUS_END);
        out << "  " << obj->dynamicState(PLUS_END);
        out << "  " << std::setw(9) << obj->posEnd(PLUS_END);
        out << "  " << std::setw(9) << obj->dirEnd(PLUS_END);
        out << std::endl;
    }
}


/**
 Export Fiber-number, position of model points
 */
void Simul::reportFiberPoints(std::ostream& out) const
{
    out << "% id pos_x pos_y pos_z" << std::endl;
    
    // we print Fibers in the order of the inventory:
    Fiber * fib = static_cast<Fiber*>(fibers.inventory.first());
    
    while ( fib )
    {
        out << "% fiber " << fib->reference() << std::endl;
        for ( unsigned p = 0; p < fib->nbPoints(); ++p )
        {
            out << std::endl << " " << fib->number();
            out << " " << std::setw(9) << fib->posPoint(p);
        }
        out << std::endl;
        fib = static_cast<Fiber*>(fibers.inventory.next(fib));
    }
}

/**
 Export Fiber-number, position of model points
 */
void Simul::reportFiberForces(std::ostream& out) const
{
    computeForces();
    
    out << "% id pointI pointX pointY ... " << std::endl;
    out << std::fixed;
    out.precision(6);
    
    // we print Fibers in the order of the inventory:
    Fiber * fib = static_cast<Fiber*>(fibers.inventory.first());
    while ( fib )
    {
        if ( fib )
        {
            out << "% fiber " << fib->reference();
            write(out, fib);
            out << std::endl;
            
            for ( unsigned p = 0; p < fib->nbPoints(); ++p )
            {
                out << std::left << std::setw(4) << fib->number();
                out << "   " << std::setw(4) << p;
                out << "   " << std::setw(10) << fib->posPoint(p);
                out << "   " << std::setw(10) << fib->netForce(p);
                if ( p == fib->lastPoint() )
                    out << "   " << std::setw(10) << 0.0;
                else
                    out << "   " << std::setw(10) << fib->tension(p);
                out << std::endl;
            }
            out << std::endl;
        }
        fib = static_cast<Fiber*>(fibers.inventory.next(fib));
    }
}



/**
 Export positions of points taken randomly along all fibers,
 but that remain static with respect to the lattice of each fiber,
 during the life-time of this fiber.
 
 This is meant to simulate the `speckle microscopy` that is obtained
 in microcscopy with a low amount of fluorescent-monomers.
 
 The distance between the speckles follows an exponential distribution
 with an average defined by the parameter `spread`.
 */
void Simul::reportFiberSpeckles(std::ostream& out, real spread) const
{
    const real tiny = 0x1p-32;
    
    out << std::fixed;
    out.precision(6);
    
    Fiber * fib = fibers.first();
    while ( fib )
    {
        out << std::endl << "% fiber " << fib->reference() << std::endl;
        
        // generate speckles below the origin of abscissa
        if ( fib->abscissaM() < 0 )
        {
            uint32_t z = fib->signature();
            real a = spread * log(z*tiny);
            while ( a > fib->abscissaP() )
            {
                z = lcrng2(z);
                a += spread * log(z*tiny);
            }
            while ( a >= fib->abscissaM() )
            {
                out << fib->pos(a) << std::endl;
                z = lcrng2(z);
                a += spread * log(z*tiny);
            }
        }
        // generate speckles above the origin of abscissa
        if ( fib->abscissaP() > 0 )
        {
            uint32_t z = ~fib->signature();
            real a = -spread * log(z*tiny);
            while ( a < fib->abscissaM() )
            {
                z = lcrng1(z);
                a -= spread * log(z*tiny);
            }
            while ( a <= fib->abscissaP() )
            {
                out << fib->pos(a) << std::endl;
                z = lcrng1(z);
                a -= spread * log(z*tiny);
            }
        }
        
        fib = fib->next();
    }
}

/**
 Sum of the internal tensions from fiber segments that intersect a plane
 specified in `opt`.
 The plane is defined by <em> n.x + a = 0 </em>
 @code
 plane = NORMAL, SCALAR
 @endcode
 */
void Simul::reportFiberTension(std::ostream& out, Glossary& opt) const
{
    computeForces();
    
    real a = 0;
    Vector n(1,0,0);
    real ten = 0;
    unsigned cnt = 0;
    if ( opt.set(n, "plane") )
    {
        opt.set(a, "plane", 1);
        fibers.infoTension(cnt, ten, n, a);
    }
    else
    {
        fibers.infoTension(cnt, ten);
    }
    
    out << "% plane (" << n << ").P + " << a << " = 0" << std::endl;
    out << "% cnt force" << std::endl;
    out << std::fixed;
    out.precision(6);
    out << std::setw(5) << cnt << " " << std::setw(8) << ten << "  " << std::endl;
}


#pragma mark -

/**
 Export position of Asters and other organizers
 */
void Simul::reportOrganizer(std::ostream& out) const
{
    out << "% class id position" << std::endl;
    out.precision(5);
    
    for ( Organizer * obj=organizers.first(); obj; obj=obj->next() )
    {
        write(out, obj);
        out << "   " << std::setw(9) << obj->position();
        out << "   " << obj->nbOrganized();
        out << std::endl;
    }
}

/**
 Export position of Beads
 */
void Simul::reportBeadPosition(std::ostream& out) const
{
    out << "% class id position" << std::endl;
    
    for ( Bead * obj=beads.first(); obj; obj=obj->next() )
    {
        write(out, obj);
        out << "   " << obj->position() << std::endl;
    }
}


/**
 Export number of beads classified as a function of
 the number of grafted Single that are attached to Fibers
 */
void Simul::reportBeadSingles(std::ostream& out) const
{
    out << "% beads: amount(nb_attached_hands)" << std::endl;
    
    std::map<Number, int> cnt;
    
    for ( Single * sig=singles.firstA(); sig; sig=sig->next() )
    {
        Mecable const* mec = sig->foot();
        if ( mec && mec->tag() == Bead::TAG )
            ++cnt[ mec->number() ];
    }
    
    const int max = 12;
    int nb[max] = { 0 };
    for ( Bead * obj=beads.first(); obj; obj=obj->next() )
        ++nb[ cnt[obj->number()] ];
    
    for ( int c = 0; c < max; ++c )
        out << " " << std::setw(3) << nb[c];
    out << std::endl;
}


/**
 Export position of Solids
 */
void Simul::reportSolid(std::ostream& out) const
{
    out << "% class id position_centroid, point_0, point_1" << std::endl;
    for ( Solid * obj=solids.first(); obj; obj=obj->next() )
    {
        write(out, obj);
        out << "   " << obj->centroid();
        out << "   " << obj->posPoint(0);
        if ( obj->nbPoints() > 1 )
            out << "   " << obj->posPoint(1);
        
        if ( modulo )
        {
            Vector pos = obj->centroid();
            modulo->fold(pos);
            out << "   " << pos;
        }
        out << std::endl;
    }
}

/**
 Export position of Sphere
 */
void Simul::reportSphere(std::ostream& out) const
{
    out << "% class id position point_1" << std::endl;
    
    for ( Sphere * obj=spheres.first(); obj; obj=obj->next() )
    {
        write(out, obj);
        out << "   " << obj->posPoint(0);
        if ( obj->nbPoints() > 1 )
            out <<  "   " << obj->posPoint(1);
        out << std::endl;
    }
}



/**
 Export something about Space
 */
void Simul::reportSpace(std::ostream& out) const
{
    out << "% class id " << std::endl;
    
    for ( Space * obj=spaces.first(); obj; obj=obj->next() )
    {
        write(out, obj);
        out << std::endl;
    }
}


#pragma mark -


void write(std::ostream& out, Couple * obj, const char str[])
{
    out << obj->property()->index() << " " << std::setw(8) << obj->number();
    out << str << "  " << obj->position() << std::endl;
}

/**
 Export position of Couples
 */
void Simul::reportCouplePosition(std::ostream& out) const
{
    out << "% class id state1 state2 position" << std::endl;
    
    for ( Couple * obj=couples.firstFF(); obj ; obj = obj->next() )
        write(out, obj, " 0 0 ");
    
    for ( Couple * obj=couples.firstAF(); obj ; obj = obj->next() )
        write(out, obj, " 1 0 ");
    
    for ( Couple * obj=couples.firstFA(); obj ; obj = obj->next() )
        write(out, obj, " 0 1 ");
    
    for ( Couple * obj=couples.firstAA(); obj ; obj = obj->next() )
        write(out, obj, " 1 1 ");
}

/**
 Export position of Couples of a certain kind
 */
void Simul::reportCouplePosition(std::ostream& out, std::string const& who) const
{
    Property * prop = properties.find("couple", who);
    
    if ( prop == 0 )
        throw InvalidParameter("Unknown couple `"+who+"'");
    
    out << "% class id state1 state2 position" << std::endl;
    
    for ( Couple * obj=couples.firstFF(); obj ; obj = obj->next() )
        if ( obj->property() == prop )
            write(out, obj, " 0 0 ");
    
    for ( Couple * obj=couples.firstAF(); obj ; obj = obj->next() )
        if ( obj->property() == prop )
            write(out, obj, " 1 0 ");
    
    for ( Couple * obj=couples.firstFA(); obj ; obj = obj->next() )
        if ( obj->property() == prop )
            write(out, obj, " 0 1 ");
    
    for ( Couple * obj=couples.firstAA(); obj ; obj = obj->next() )
        if ( obj->property() == prop )
            write(out, obj, " 1 1 ");
}


/**
 Export state of Couples
 */
void Simul::reportCouple(std::ostream& out) const
{
    PropertyList plist = properties.find_all("couple");
    
    const unsigned mx = 128;
    int nb[mx] = { 0 }, cnt[mx][4];
    
    //reset counts:
    for ( int ii = 0; ii < mx; ++ii )
    {
        cnt[ii][0] = 0;
        cnt[ii][1] = 0;
        cnt[ii][2] = 0;
        cnt[ii][3] = 0;
    }
    
    for ( Couple * cxi=couples.firstFF(); cxi ; cxi = cxi->next() )
    {
        unsigned ix = cxi->property()->index();
        if ( ix < mx )
        {
            ++nb[ix];
            ++(cnt[ix][0]);
        }
    }
    
    for ( Couple * cxi=couples.firstAF(); cxi ; cxi = cxi->next() )
    {
        unsigned ix = cxi->property()->index();
        if ( ix < mx )
        {
            ++nb[ix];
            ++(cnt[ix][1]);
        }
    }
    for ( Couple * cxi=couples.firstFA(); cxi ; cxi = cxi->next() )
    {
        unsigned ix = cxi->property()->index();
        if ( ix < mx )
        {
            ++nb[ix];
            ++(cnt[ix][2]);
        }
    }
    
    for ( Couple * cxi=couples.firstAA(); cxi ; cxi = cxi->next() )
    {
        unsigned ix = cxi->property()->index();
        if ( ix < mx )
        {
            ++nb[ix];
            ++(cnt[ix][3]);
        }
    }
    
    if ( 1 )
    {
        out << std::setw(16) << std::left << "% Couple" << std::right;
        out << " " << std::setw(8) << "Total";
        out << " " << std::setw(8) << "FF";
        out << " " << std::setw(8) << "AF";
        out << " " << std::setw(8) << "FA";
        out << " " << std::setw(8) << "AA";
        out << std::endl;
    }
    
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        out << std::setw(16) << std::left << (*ip)->name() << std::right;
        unsigned ix = (*ip)->index();
        if ( ix < mx )
        {
            out << " " << std::setw(8) << nb[ix];
            for ( unsigned int d = 0; d < 4; ++d )
                out << " " << std::setw(8) << cnt[ix][d];
            out << std::endl;
        }
        else
            out << " out-of-range " << std::endl;
    }
}


#pragma mark -

void write(std::ostream& out, Single * obj, const char str[])
{
    out << obj->property()->index();
    out << "  " << std::setw(8) << obj->number();
    out << str  << std::setw(8) << obj->position();
    if ( obj->attached() )
        out << "  " << std::setw(8) << obj->force();
    else
        out << "  " << Vector(0,0,0) << std::setw(8) ;
    out << std::endl;
}


/**
 Export position of Singles
 */
void Simul::reportSinglePosition(std::ostream& out) const
{
    out << "% class id state position force" << std::endl;
    
    for ( Single * obj=singles.firstF(); obj ; obj = obj->next() )
        write(out, obj, " 0  ");
    
    for ( Single * obj=singles.firstA(); obj ; obj = obj->next() )
        write(out, obj, " 1  ");
}

/**
 Export position of Singles
 */
void Simul::reportSingleForce(std::ostream& out) const
{
    out << "% class id state position force" << std::endl;
    
    for ( Single * obj=singles.firstA(); obj ; obj = obj->next() )
        write(out, obj, " 1  ");
}


/**
 Export position of Singles
 */
void Simul::reportSinglePosition(std::ostream& out, std::string const& who) const
{
    Property * prop = properties.find("single", who);
    
    if ( prop == 0 )
        throw InvalidParameter("Unknown single `"+who+"'");
    
    out << "% class id state position force" << std::endl;
    
    for ( Single * obj=singles.firstF(); obj ; obj = obj->next() )
        if ( obj->property() == prop )
            write(out, obj, " 0  ");
    
    for ( Single * obj=singles.firstA(); obj ; obj = obj->next() )
        if ( obj->property() == prop )
            write(out, obj, " 1  ");
}


/**
 Export number of Single in each state
 */
void Simul::reportSingle(std::ostream& out) const
{
    PropertyList plist = properties.find_all("single");
    
    const unsigned mx = 128;
    
    int nb[mx] = { 0 }, free[mx] = { 0 }, bound[mx] = { 0 };
    
    for ( Single * si = singles.firstF(); si ; si = si->next() )
    {
        unsigned ix = si->property()->index();
        if ( ix < mx )
        {
            ++nb[ix];
            ++free[ix];
        }
    }
    
    for ( Single * si = singles.firstA(); si ; si = si->next() )
    {
        unsigned ix = si->property()->index();
        if ( ix < mx )
        {
            ++nb[ix];
            ++bound[ix];
        }
    }
    
    if ( 1 )
    {
        out << std::setw(16) << std::left << "% Single" << std::right;
        out << " " << std::setw(8) << "Total";
        out << " " << std::setw(8) << "Free";
        out << " " << std::setw(8) << "Bound";
        out << std::endl;
    }
    
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        out << std::setw(16) << std::left << (*ip)->name() << std::right;
        unsigned ix = (*ip)->index();
        if ( ix < mx )
        {
            out << " " << std::setw(8) << nb[ix];
            out << " " << std::setw(8) << free[ix];
            out << " " << std::setw(8) << bound[ix] << std::endl;
        }
        else
            out << " out-of-range " << std::endl;
    }
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Substitute the values of fiber:fleck() such that `a` and `b`
 values are replaced by the smallest between `a` and `b`.
 */
void reFleck(FiberSet const& set, int a, int b)
{
    if ( b < a )
    {
        for ( Fiber* fib = set.first(); fib; fib=fib->next() )
        {
            if ( fib->fleck() == a )
                fib->fleck(b);
        }
    }
    else
    {
        for ( Fiber* fib = set.first(); fib; fib=fib->next() )
        {
            if ( fib->fleck() == b )
                fib->fleck(a);
        }
    }
}


/**
 Set Fiber::fleck to indicated Fibers that are connected by Couple.
 
 The clusters are defined by the Couple that are bridging Fibers:
 Two fibers are in the same cluster if there is a Couple connecting them,
 of if they can be indirectly connected in this way via other Fibers.
 
 This analysis can be useful to identify mechanically isolated sub-networks
 in the simulation.
 The result can be visualized in `play` with the option fiber:coloring=4,
 and it can also be printed with the tool `report fiber:cluster`
 */
void Simul::analyzeClusters() const
{
    // set unique fleck() for each fiber:
    for ( Fiber * fib = fibers.first(); fib; fib=fib->next() )
        fib->fleck(fib->number());
    
    // equalize fleck() when fibers are connected by a Couple:
    for ( Couple * cx = couples.firstAA(); cx ; cx=cx->next() )
    {
        if ( cx->fiber1()->fleck() != cx->fiber2()->fleck() )
            reFleck(fibers, cx->fiber1()->fleck(), cx->fiber2()->fleck());
    }
}



/**
 Export size of clusters found by Simul::analyzeClusters()
 */
void Simul::reportClusters(std::ostream& out, bool details) const
{
    analyzeClusters();
    
    typedef std::map<int, std::set<Number> > map_t;
    map_t map;
    
    for ( Fiber * fib = fibers.first(); fib; fib=fib->next() )
        map[fib->fleck()].insert(fib->number());
    
    out << "% cluster-index, number-of-fibers : name-of-fibers" << std::endl;
    for ( map_t::const_iterator mi = map.begin(); mi != map.end(); ++mi )
    {
        out << (*mi).first << "  " << (*mi).second.size() << " :";
        if ( details )
        {
            for ( std::set<Number>::const_iterator si = (*mi).second.begin(); si != (*mi).second.end(); ++si )
                out << " " << (*si);
        }
        out << std::endl;
    }
}

#pragma mark -


/**
 Export end-to-end distance of Fiber
 */
void Simul::reportCustom(std::ostream& out) const
{
    out.precision(6);
    
    for ( Fiber * obj=fibers.first(); obj; obj=obj->next() )
    {
        Vector ee = obj->posEnd(PLUS_END) - obj->posEnd(MINUS_END);
        out << ee.norm() << " ";
    }
    out << std::endl;
}

