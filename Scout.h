#ifndef SCOUT_H
#define SCOUT_H

/*! \file
 * Defintion of the Scout class.
 * by Jannis Warnat
 */

CGAL_BEGIN_NAMESPACE

template<class Arrangement_, class Traits_>
  class Scout
  {
  public:

    typedef Traits_ Traits_2;
    typedef Arrangement_ Arrangement_2;
    typedef My_Sweep_line_subcurve<Arrangement_2, Traits_2> Subcurve;
    typedef My_Sweep_line_2<Arrangement_2, Traits_2> Sweep_line;
typedef    typename Sweep_line::Scout_set Scout_set;
    typedef typename Sweep_line::Scout_set_iterator Scout_set_iterator;
    typedef typename Sweep_line::Scout_set_reverse_iterator Scout_set_reverse_iterator;
    typedef typename Traits_2::Point_2 Point_2;
    typedef typename Arrangement_2::Vertex_const_handle Vertex_const_handle;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef Scout<Arrangement_2,Traits_2> Self;

    //pointer to the scout set
    Scout_set* m_scouts;

    //pointer to the sweep line
    Sweep_line* sweep_line;

  private:

    //the boundary curve the scout moves on
    Subcurve* b_cv;

    //the curve the scout is guarding
    Subcurve* g_cv;

    //the partner in enclosing a region
    Self* partn;

    //a scout can be idled or active
    bool i_active;

    //iterator to the scout in the scout set for efficient insertion and erasure
    Scout_set_iterator scout_set_pos;

  public:

    enum Scout_type
      {
        UPPER = 0,

        LOWER = 1,

        UPPER_DUMMY = 2,

        LOWER_DUMMY = 4
      };

    Scout_type s_type;

    //color of a scout is the color of its boundary curve
    typename Traits_2::Color col;

    Scout ()
      {

      }

    //standard constructor
    Scout(Subcurve* b, Scout_type st, Sweep_line* sl)
      {
        sweep_line = sl;
        m_scouts = sweep_line->get_scouts();
        i_active = false;
        partn = 0;
        b_cv = b;
        col = b_cv->color();
        g_cv = 0;
        s_type = st;

        //set the pointers at the boundary curve
        if(s_type == UPPER || s_type == UPPER_DUMMY)
          {
            b_cv->set_upper_scout(this);
          }

        if(s_type == LOWER || s_type == LOWER_DUMMY)
          {
            b_cv->set_lower_scout(this);
          }

        Scout_set_iterator ssi;
        if(s_type == LOWER_DUMMY)
          {
            ssi = m_scouts->insert_before(m_scouts->begin(),this);
          }

        if(s_type == UPPER_DUMMY)
          {
            ssi = m_scouts->insert_before(m_scouts->end(),this);
          }

        if(s_type == LOWER)
          {
            ssi = m_scouts->insert(this);
          }
        scout_set_pos = ssi;
      }

    //insert an upper scout efficiently right above its lower scout partner
    Scout(Subcurve* b, Scout_type st, Sweep_line* sl, Scout_set_iterator lower)
      {
        sweep_line = sl;
        m_scouts = sweep_line->get_scouts();
        i_active = false;
        partn = 0;
        b_cv = b;
        col = b_cv->color();
        g_cv = 0;
        s_type = st;
        b_cv->set_upper_scout(this);
        Scout_set_iterator ssi = m_scouts->insert_before(lower,this);
        scout_set_pos = ssi;
      }

    ~Scout ()
      {

      }

    void set_partner(Self* s)
      {
        partn = s;
      }

    Self* partner()
      {
        return partn;
      }

    Scout_set_iterator scout_set_position()
    {
      return scout_set_pos;
    }

    void set_scout_set_position(Scout_set_iterator ssi)
    {
      scout_set_pos = ssi;
    }

    Subcurve* guarded_curve()
      {
        return g_cv;
      }

    Subcurve* boundary_curve()
      {
        return b_cv;
      }

    typename Traits_2::Color color()
      {
        return col;
      }

    bool is_active()
      {

        return i_active;
      }

    bool is_idled()
      {
        return !i_active;
      }

    void set_active()
      {
        i_active = true;
        if(this->is_guarding())
          {
            //set the pointers at the guarded curve
            if(this->type() == UPPER)
              {
                this->guarded_curve()->set_upper_guard(this);
              }

            if(this->type() == LOWER)
              {
                this->guarded_curve()->set_lower_guard(this);
              }
          }
      }

    void set_idled()
      {
        i_active = false;
        set_guarded_curve(0);

        //remove the pointers at the guarded curve
        if(this->type() == UPPER && this->is_guarding())
          {
            this->guarded_curve()->remove_upper_guard();
          }

        if(this->type() == LOWER && this->is_guarding())
          {
            this->guarded_curve()->remove_lower_guard();
          }
      }

    // ---------------------------------------------------------------------------
    // insert the boundary curve the scout moves on into the purple
    // arrangement

    void insert_into_purple_arr()
      {
        if(color() == Traits_2::DUMMY)
          {
            return;
          }

        //insert every subcurve only once
        if(!boundary_curve()->is_in_purple())
          {
            Point_2 left = boundary_curve()->last_curve().left();
            Point_2 right = boundary_curve()->last_curve().right();

            //the vertices are already in the purple arrangement;
            //get these
            Vertex_const_handle left_vh;
            try
              {
                left_vh = object_cast<Vertex_const_handle> (
                    left.purple_object());
              }
            catch (Bad_object_cast)
              {
                CGAL_assertion(false);
              }

            Vertex_const_handle right_vh;
            try
              {
                right_vh = object_cast<Vertex_const_handle> (
                    right.purple_object());
              }
            catch (Bad_object_cast)
              {
                CGAL_assertion(false);
              }

            //convert to non-const
            Vertex_handle left_n_vh = sweep_line->get_purple()->non_const_handle(left_vh);
            Vertex_handle right_n_vh = sweep_line->get_purple()->non_const_handle(right_vh);

            //insert efficiently
            sweep_line->get_purple()->insert_at_vertices(boundary_curve()->last_curve(),left_n_vh,right_n_vh);

            //make sure no curve is inserted twice
            boundary_curve()->set_in_purple(true);
          }
      }

    // ---------------------------------------------------------------------------
    // check for an intersection between the boundary curve and the
    // guarded curve at a scout; use functionality of the CGAL sweep
    // line framework

    void
    check_individual_for_intersection()
      {
        if (color() == Traits_2::RED)
          {
            sweep_line->_intersect_from_red(boundary_curve(),
                guarded_curve());
          }
        else
          {
            if(color() == Traits_2::BLUE)
              {
                sweep_line->_intersect_from_blue(boundary_curve(),
                    guarded_curve());
              }
          }
      }

    // ---------------------------------------------------------------------------
    // checks for an intersection of the boundary curves of two partner scouts

      void
      check_partners_for_intersection()
      {
        if (color() == Traits_2::RED)
          {
            if (partner()->color() == Traits_2::BLUE)
              {
                sweep_line->_intersect_from_red(boundary_curve(),
                    partner()->boundary_curve());
              }
          }
        else
          {
            if (color() == Traits_2::BLUE)
              {
                if (partner()->color() == Traits_2::RED)
                  {
                    sweep_line->_intersect_from_blue(boundary_curve(),
                        partner()->boundary_curve());
                  }
              }
          }
      }

    // ---------------------------------------------------------------------------
    // A lower scout has been started; check if the highest lower scout below
    // of the same color
    // already guards the curve to guard. If so, set the new scout idled.
    //

    void
    start_lower_inform_lower_below()
      {
        MY_CGAL_SL_PRINT("begin start_lower_inform_lower_below()");
        Scout_set_iterator self = scout_set_position();

        //if we find no scout to idle self, it will be activated
        bool activate_self = true;

        //walk down the scout set
        Scout_set_iterator l_minus = self;
        while(l_minus != m_scouts->begin())
          {
            //skip upper scouts
            l_minus--;
            l_minus--;

            if((*l_minus)->color() == (*self)->color())
              {
                //we must go down until we find an active one,
                //the scouts in between might be idled
                if((*l_minus)->is_active())
                  {
                    if((*l_minus)->guarded_curve() == (*self)->guarded_curve())
                      {
                        activate_self = false;
                      }
                    //either way, we can break now
                    break;
                  }
              }
          }

        if(activate_self)
          {
            (*self)->set_active();
            (*self)->check_individual_for_intersection();
          }
        else
          {
            (*self)->set_idled();
          }
      }

    // ---------------------------------------------------------------------------
    // A lower scout has been started; check if the lowest lower scout above
    // of the same color must be set idled.
    //

    void
    start_lower_inform_lower_above()
      {
        MY_CGAL_SL_PRINT("begin start_lower_inform_lower_above()");
        Scout_set_iterator self = scout_set_position();

        //walk up the scout set
        Scout_set_iterator l_plus = self;
        do
          {
            //skip upper scouts
            l_plus++;
            l_plus++;

            if(l_plus != m_scouts->end())
              {
                if((*l_plus)->color() == (*self)->color())
                  {
                    if((*l_plus)->guarded_curve() == (*self)->guarded_curve())
                      {
                        (*l_plus)->set_idled();
                      }
                    break;
                  }
              }
          }while(l_plus != m_scouts->end());
      }

    // ---------------------------------------------------------------------------
    // An upper scout has been started; check if the highest upper scout below
    // of the same color must be set idled.
    //

    void
    start_upper_inform_upper_below()
      {
        MY_CGAL_SL_PRINT("begin start_upper_inform_upper_below()");
        Scout_set_reverse_iterator self = Scout_set_reverse_iterator(scout_set_position());
        self--;

        //walk up the scout set
        Scout_set_reverse_iterator u_minus = self;
        do
          {
            //skip lower scouts
            u_minus++;
            u_minus++;

            if(u_minus != m_scouts->rend())
              {
                if((*u_minus)->color() == (*self)->color())
                  {
                    if((*u_minus)->guarded_curve() == (*self)->guarded_curve())
                      {
                        (*u_minus)->set_idled();
                      }
                    break;
                  }
              }
          }while(u_minus != m_scouts->rend());
      }

    // ---------------------------------------------------------------------------
    // A upper scout has been started; check if the lowest upper scout above
    // of the same color already guards the curve to guard.
    // If so, set the new scout idled.
    //

    void
    start_upper_inform_upper_above(
    )
      {
        MY_CGAL_SL_PRINT("begin start_upper_inform_upper_above()");
        Scout_set_reverse_iterator self = Scout_set_reverse_iterator(scout_set_position());
        self--;

        //if we find no scout to idle self, it will be activated
        bool activate_self = true;

        //walk up the scout set
        Scout_set_reverse_iterator u_plus = self;
        while(u_plus != m_scouts->rbegin())
          {
            //skip lower scouts
            u_plus--;
            u_plus--;

            if((*u_plus)->color() == (*self)->color())
              {
                //we must go up until we find an active one,
                //the scouts in between might be idled
                if((*u_plus)->is_active())
                  {
                    if((*u_plus)->guarded_curve() == (*self)->guarded_curve())
                      {
                        activate_self = false;
                      }
                    //either way, we can break now
                    break;
                  }
              }
          }

        if(activate_self)
          {
            (*self)->set_active();
            (*self)->check_individual_for_intersection();
          }
        else
          {
            (*self)->set_idled();
          }
      }

    // ---------------------------------------------------------------------------
    // A lower scout has been ended; check if the lowest lower scout above
    // of the same color was idled and must now be activated.
    //

    void
    end_lower_inform_lower_above()
      {
        MY_CGAL_SL_PRINT("begin end_lower_inform_lower_above()");

        Scout_set_iterator self = scout_set_position();

        //if self is idled the "idler" will also idle the lowest lower scout above
        if((*self)->is_idled())
          {
            return;
          }

        //walk up the scout set
        Scout_set_iterator l_plus = self;
        do
          {
            //skip upper scouts
            l_plus++;
            l_plus++;

            if(l_plus != m_scouts->end())
              {
                if((*l_plus)->color() == (*self)->color())
                  {
                    if ((*l_plus)->is_idled())
                      {
                        (*l_plus)->set_active();
                        (*l_plus)->set_guarded_curve((*self)->guarded_curve());
                        (*self)->set_idled();
                        (*l_plus)->check_individual_for_intersection();
                      }
                    break;
                  }
              }
          }while(l_plus != m_scouts->end());
      }

    // ---------------------------------------------------------------------------
    // An upper scout has been ended; check if the highest upper scout below
    // of the same color was idled and must now be activated
    //

    void
    end_upper_inform_upper_below()
      {
        MY_CGAL_SL_PRINT("begin end_upper_inform_upper_below()");
        Scout_set_reverse_iterator self = Scout_set_reverse_iterator(scout_set_position());
        self--;

        //if self is idled the "idler" will also idle the highest upper scout below
        if((*self)->is_idled())
          {
            return;
          }

        //walk down the scout set
        Scout_set_reverse_iterator u_minus = self;
        do
          {
            u_minus++;
            u_minus++;

            if(u_minus != m_scouts->rend())
              {
                if((*u_minus)->color() == (*self)->color())
                  {
                    if ((*u_minus)->is_idled())
                      {
                        (*u_minus)->set_active();
                        (*u_minus)->set_guarded_curve((*self)->guarded_curve());
                        (*self)->set_idled();
                        (*u_minus)->check_individual_for_intersection();
                      }
                    break;
                  }
              }
          }while(u_minus != m_scouts->rend());
      }

    const Scout_type type() const
      {
        return s_type;
      }

    bool is_guarding()
      {
        if(g_cv != 0)
          {
            return true;
          }
        else
          {
            return false;
          }
      }

    void set_guarded_curve(Subcurve* g)
      {
        //if we guard a curve already, release it
        if(g_cv != 0)
          {
            if(s_type == LOWER && this->is_active())
              {
                g_cv->remove_lower_guard();
              }
            if(s_type == UPPER && this->is_active())
              {
                g_cv->remove_upper_guard();
              }
          }

        g_cv = g;

        //set this as the new guard for the curve
        if(g_cv != 0)
          {
            if(s_type == LOWER && this->is_active())
              {
                g_cv->set_lower_guard(this);
              }
            if(s_type == UPPER && this->is_active())
              {
                g_cv->set_upper_guard(this);
              }
          }
      }

    void set_boundary_curve(Subcurve* b)
      {
        b_cv = b;
        col = b_cv->color();
      }

    void delete_upper_scout()
      {
        if(this == this->sweep_line->upper_dummy)
          {
            this->sweep_line->upper_dummy = 0;
          }

        //remove the pointers at the guarded curve
        if(this->is_active() && this->is_guarding())
          {
            this->guarded_curve()->remove_upper_guard();
          }

        //erase from the scout set
        this->m_scouts->erase(scout_set_pos);
        this->boundary_curve()->set_upper_scout(0);
      }

    void delete_lower_scout()
      {
        if(this == this->sweep_line->lower_dummy)
          {
            this->sweep_line->lower_dummy = 0;
          }

        //remove the pointers at the guarded curve
        if(this->is_active() && this->is_guarding())
          {
            this->guarded_curve()->remove_lower_guard();
          }

        //erase from the scout set
        this->m_scouts->erase(scout_set_pos);
        this->boundary_curve()->set_lower_scout(0);
      }


    void Print()
      {
        std::cout << "Scout " << this;

        if(this->is_active())
          {
            std::cout << " active";
          }
        else
          {
            std::cout << " idled";
          }

        if(this->type() == UPPER)
          {
            std::cout << " U ";
          }
        if(this->type() == LOWER)
          {
            std::cout << " L ";
          }
        if(this->type() == LOWER_DUMMY)
          {
            std::cout << " LD ";
          }
        if(this->type() == UPPER_DUMMY)
          {
            std::cout << " UD ";
          }

        std::cout << " on " << this->boundary_curve()->last_curve();

        if(this->is_guarding())
          {
            std::cout << " guarding " << this->guarded_curve()->last_curve();
          }
        else
          {
            std::cout << "guarding NOTHING ";
          }

        std::cout << "" << std::endl;

      }

  };

CGAL_END_NAMESPACE

#endif
