using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class otherEllenController : MonoBehaviour {

    private CharacterController controller;
    private Animator animator;
    public float moveSpeed = 15.0f;
    public float animationSpeed = 8.0f;
    public bool moveFlag= false;
    public bool attackFlag = false;
    public bool comboFlag = false;
    public float idleTimeout = 5f;
    public float idleTimer = 0f;
    public Quaternion targetRotation;
    public Gamekit3D.MeleeWeapon meleeWeapon;
    public hpController hpBar;

    public System.UInt64 Index;
    public int Level;
    public int currentHP, maxHP;

    // Parameters
    readonly int m_HashForwardSpeed = Animator.StringToHash("ForwardSpeed");
    readonly int m_HashMeleeAttack = Animator.StringToHash("MeleeAttack");
    readonly int m_HashTimeoutToIdle = Animator.StringToHash("TimeoutToIdle");
    readonly int m_HashDeath = Animator.StringToHash("Death");
    readonly int m_HashHurt = Animator.StringToHash("Hurt");
    readonly int m_HashRespawn = Animator.StringToHash("Respawn");
    readonly int m_HashHurtFromX = Animator.StringToHash("HurtFromX");
    readonly int m_HashHurtFromY = Animator.StringToHash("HurtFromY");
    readonly int m_HashInputDetected = Animator.StringToHash("InputDetected");
    readonly int m_HashStateTime = Animator.StringToHash("StateTime");

    // States
    readonly int m_HashLocomotion = Animator.StringToHash("Locomotion");
    readonly int m_HashAirborne = Animator.StringToHash("Airborne");
    readonly int m_HashLanding = Animator.StringToHash("Landing");    // Also a parameter.
    readonly int m_HashEllenCombo1 = Animator.StringToHash("EllenCombo1");
    readonly int m_HashEllenCombo2 = Animator.StringToHash("EllenCombo2");
    readonly int m_HashEllenCombo3 = Animator.StringToHash("EllenCombo3");
    readonly int m_HashEllenCombo4 = Animator.StringToHash("EllenCombo4");
    readonly int m_HashEllenDeath = Animator.StringToHash("EllenDeath");

    // Use this for initialization
    private void Awake()
    {
        controller = GetComponent<CharacterController>();
        animator = GetComponent<Animator>();
        meleeWeapon = gameObject.GetComponentInChildren<Gamekit3D.MeleeWeapon>();
        meleeWeapon.SetOwner(gameObject);
        targetRotation = transform.rotation;
    }

    void TimeoutToIdle()
    {
        bool inputDetected = moveFlag || attackFlag;
        if (!inputDetected)
        {
            idleTimer += Time.deltaTime;

            if (idleTimer >= idleTimeout)
            {
                idleTimer = 0f;
                animator.SetTrigger(m_HashTimeoutToIdle);
            }
        }
        else
        {
            idleTimer = 0f;
            animator.ResetTrigger(m_HashTimeoutToIdle);
        }
        animator.SetBool(m_HashInputDetected, inputDetected);
    }

    public void setMove(Quaternion _transforward, bool _val)
    {
        targetRotation = _transforward;
        moveFlag = _val;
    }

    // This is called by an animation event when Ellen swings her staff.
    public void MeleeAttackStart(int throwing = 0)
    {
        meleeWeapon.BeginAttack(throwing != 0);
        attackFlag = true;
    }

    // This is called by an animation event when Ellen finishes swinging her staff.
    public void MeleeAttackEnd()
    {
        meleeWeapon.EndAttack();
        attackFlag = false;
    }

    public void setAttack(Quaternion _transforward, bool _val)
    {
        targetRotation = _transforward;
        attackFlag = _val;
        if (attackFlag)
            animator.SetTrigger(m_HashMeleeAttack);
    }

    private void Move()
    {
        Vector3 movement = new Vector3(0, -1f, 0);
        if (moveFlag)
        {
            movement += transform.forward.normalized * moveSpeed * Time.deltaTime;
            animator.SetFloat(m_HashForwardSpeed, animationSpeed);
        }
        else
            animator.SetFloat(m_HashForwardSpeed, 0f);
        controller.Move(movement);
        animator.SetBool("Grounded", true);
    }

    private void Attack()
    {
        bool attacking = attackFlag | comboFlag;
        comboFlag = attackFlag;
        attackFlag = false;

        if (!attacking)
            animator.ResetTrigger(m_HashMeleeAttack);
    }

    private void Update()
    {
        animator.SetFloat(m_HashStateTime, Mathf.Repeat(animator.GetCurrentAnimatorStateInfo(0).normalizedTime, 1f));

        transform.rotation = targetRotation;

        TimeoutToIdle();

        Attack();

    }

    private void OnAnimatorMove()
    {
        Move();
    }

    public void setIndex(System.UInt64 _Index)
    {
        Index = _Index;
    }

    public System.UInt64 getIndex()
    {
        return Index;
    }

    public void proc_Damage(Vector3 _attackerPosition, int _damage, int _HP)
    {
        currentHP = _HP;
        hpBar.setHP(_HP);
        if (_HP <= 0)
        {
            animator.SetTrigger(m_HashDeath);
            return;
        }
        
        animator.SetTrigger(m_HashHurt);

        Vector3 forward = _attackerPosition - transform.position;
        forward.y = 0;

        Vector3 localHurt = transform.InverseTransformDirection(forward);

        animator.SetFloat(m_HashHurtFromX, localHurt.x);
        animator.SetFloat(m_HashHurtFromY, localHurt.z);
    }

    public void proc_Recovery(int _recovertAmount, int _currentHP)
    {
        currentHP = _currentHP;
        hpBar.setHP(_currentHP);
    }

    public void setPlayerData(int _Level, int _maxHP, int _currentHP, hpController _hpCntler, Slider _hpSlider)
    {
        Level = _Level;
        maxHP = _maxHP;
        currentHP = _currentHP;
        hpBar = _hpCntler;
        hpBar.setController(gameObject, maxHP, currentHP, _hpSlider);
        hpBar.setHP(currentHP);
    }
}
